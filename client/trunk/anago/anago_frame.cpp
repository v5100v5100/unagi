#include <wx/wx.h>
#include <wx/log.h>
#include <wx/dir.h>
#include <wx/thread.h>
#include <wx/app.h>
#include <cstring>
#include <cstdarg>
#include "anago_gui.h"
#include "widget.h"
#include "reader_master.h"
//#include "reader_kazzo.h"
extern const struct reader_driver DRIVER_KAZZO;
extern "C"{
#include "header.h"
#include "flash_device.h"
#include "script_dump.h"
#include "script_program.h"
}

//---- C++ -> C -> C++ wrapping functions ----
static void value_set(void *gauge, void *label, int value)
{
	wxGauge *g = static_cast<wxGauge *>(gauge);
	wxStaticText *l = static_cast<wxStaticText *>(label);
	wxString str;
	str.Printf(wxT("0x%06x/0x%06x"), value, g->GetRange());
	
	wxMutexGuiEnter();
	g->SetValue(value);
	l->SetLabel(str);
	wxMutexGuiLeave();
}

static void value_add(void *gauge, void *label, int value)
{
	wxGauge *g = static_cast<wxGauge *>(gauge);
	value += g->GetValue();
	
	value_set(gauge, label, value);
}

static void range_set(void *gauge, int value)
{
	wxGauge *g = static_cast<wxGauge *>(gauge);
	if(value == 0){
		value = 1;
	}
	g->SetRange(value);
}

static void text_append_va(void *log, const char *format, va_list list)
{
	wxTextCtrl *l = static_cast<wxTextCtrl *>(log);
	wxString str;
	str.PrintfV(format, list);

	wxMutexGuiEnter();
	*l << str;
	wxMutexGuiLeave();
}

static void text_append(void *log, const char *format, ...)
{
	va_list list;
	va_start(list, format);
	text_append_va(log, format, list);
	va_end(list);
}

static void label_set(void *label, const char *format, ...)
{
	wxStaticText *l = static_cast<wxStaticText *>(label);
	wxString str;
	va_list list;
	
	va_start(list, format);
	str.PrintfV(format, list);
	va_end(list);

	wxMutexGuiEnter();
	l->SetLabel(str);
	wxMutexGuiLeave();
}

void choice_append(void *choice, const char *str)
{
	wxChoice *c = static_cast<wxChoice *>(choice);
	c->Append(wxString(str));
}

//---- script execute thread ----
class anago_frame;
class anago_dumper : public wxThread
{
private:
	anago_frame *m_frame;
	struct dump_config m_config;
protected:
	void *Entry(void);
	void OnExit()
	{
		delete [] m_config.script;
		delete [] m_config.target;
	}
public:
	anago_dumper(anago_frame *f, const struct dump_config *d) : wxThread()
	{
		m_frame = f;
		memcpy(&m_config, d, sizeof(struct dump_config));
	}
};

class anago_programmer : public wxThread
{
private:
	anago_frame *m_frame;
	struct program_config m_config;
protected:
	void *Entry(void);
	void OnExit()
	{
		delete [] m_config.script;
		delete [] m_config.target;
	}
public:
	anago_programmer(anago_frame *f, const struct program_config *d) : wxThread()
	{
		m_frame = f;
		memcpy(&m_config, d, sizeof(struct program_config));
	}
};

//---- main frame class ----
class anago_frame : public frame_main
{
private:
	wxThread *m_anago_thread;
	enum{
		STATUS_IDLE, STATUS_DUMPPING, STATUS_PROGRAMMING
	}m_status;
	void gauge_init(struct gauge *t)
	{
		t->label_set = label_set;
		t->range_set = range_set;
		t->value_set = value_set;
		t->value_add = value_add;
	}
	void script_choice_init(wxChoice *c, wxString filespec)
	{
		wxDir dir(wxGetCwd());
		wxString filename;

		c->Clear();
		if ( !dir.IsOpened() ){
			return;
		}
		bool cont = dir.GetFirst(&filename, filespec, wxDIR_FILES);
		while ( cont ){
			c->Append(filename);
			cont = dir.GetNext(&filename);
		}
		if(c->GetCount() == 0){
			*m_log << "warning: " << filespec << " script not found.\n";
		}else{
			c->Select(0);
		}
	}
//---- dump mode functions ----
	void dump_increase_init(wxChoice *c)
	{
		c->Clear();
		c->Append(wxString("x1"));
		c->Append(wxString("x2"));
		c->Append(wxString("x4"));
		c->Select(0);
	}
	int dump_increase_get(wxChoice *c)
	{
		switch(c->GetSelection()){
		case 0: return 1;
		case 1: return 2;
		case 2: return 4;
		}
		return 1;
	}
	void dump_execute(void)
	{
		struct dump_config config;
		config.cpu.gauge.bar = m_dump_cpu_gauge;
		config.cpu.gauge.label = m_dump_cpu_value;
		gauge_init(&config.cpu.gauge);

		config.ppu.gauge.bar = m_dump_ppu_gauge;
		config.ppu.gauge.label = m_dump_ppu_value;
		gauge_init(&config.ppu.gauge);
		
		config.log.object = m_log;
		config.log.append = text_append;
		config.log.append_va = text_append_va;
		config.cpu.increase = dump_increase_get(m_dump_cpu_increase);
		config.ppu.increase = dump_increase_get(m_dump_ppu_increase);
		config.progress = true;
		config.battery = m_dump_check_battery->GetValue();
		{
			wxString str_script = m_dump_script_choice->GetStringSelection();
			char *t = new char[str_script.Length() + 1];
			config.script = t;
			strncpy(t, str_script.fn_str(), str_script.Length() + 1);
		}

		{
			wxString str;
			config.mappernum = -1;
			if(m_dump_check_forcemapper->GetValue() == true){
				str = m_dump_text_forcemapper->GetValue();
				if(str.ToLong(&config.mappernum) == false){
					*m_log << "bad mapper number\n";
					return;
				}
			}
		}

		{
			wxTextCtrl *text = m_dump_romimage_picker->GetTextCtrl();
			wxString str_rom = text->GetValue();
			char *t = new char[str_rom.Length() + 1];
			if(text->IsEmpty() == true){
				*m_log << "Enter filename to ROM image\n";
				return;
			}
			config.target = t;
			strncpy(t, str_rom.fn_str(), str_rom.Length() + 1);
		}

		config.control = &DRIVER_KAZZO.control;
		config.cpu.access = &DRIVER_KAZZO.cpu;
		config.ppu.access = &DRIVER_KAZZO.ppu;

		m_dump_script_choice->Disable();
		m_dump_romimage_picker->Disable();
		m_dump_check_battery->Disable();
		m_dump_check_forcemapper->Disable();
		m_dump_button->SetLabel(wxString("cancel"));
		m_dump_text_forcemapper->Disable();
		m_dump_cpu_increase->Disable();
		m_dump_ppu_increase->Disable();

/*		if(m_anago_thread != NULL){ //???
			delete m_anago_thread;
		}*/
		m_anago_thread = new anago_dumper(this, &config);
		if(m_anago_thread->Create() != wxTHREAD_NO_ERROR){
			*m_log << "thread creating error";
		}else if(m_anago_thread->Run() != wxTHREAD_NO_ERROR){
			*m_log << "thread running error";
		}else{
			m_status = STATUS_DUMPPING;
		}
	}
	
//----- program mode functions ----
	void program_padding_init(wxChoice *c)
	{
		c->Clear();
		c->Append(wxString("full"));
		c->Append(wxString("top"));
		c->Append(wxString("bottom"));
		c->Append(wxString("empty"));
		c->Select(0);
	}
	bool program_rom_set(wxString device, int trans, struct memory *m, struct flash_device *f)
	{
		m->offset = 0;
		if(flash_device_get(device, f) == false){
			*m_log << "unknown flash memory device ";
			*m_log << device << "\n";
			return false;
		}
		switch(trans){
		case 0: 
			m->transtype = TRANSTYPE_FULL;
			break;
		case 1: 
			m->transtype = TRANSTYPE_TOP;
			break;
		case 2: 
			m->transtype = TRANSTYPE_BOTTOM;
			break;
		default: 
			m->transtype = TRANSTYPE_EMPTY;
			break;
		}
		return true;
	}

	void program_execute(void)
	{
		struct program_config f;
		
		f.cpu.gauge.bar = m_program_cpu_gauge;
		f.cpu.gauge.label = m_program_cpu_value;
		gauge_init(&f.cpu.gauge);

		f.ppu.gauge.bar = m_program_ppu_gauge;
		f.ppu.gauge.label = m_program_ppu_value;
		gauge_init(&f.ppu.gauge);
		
		f.log.object = m_log;
		f.log.append = text_append;
		f.log.append_va = text_append_va;
		
		{
			wxString str_script = m_program_script_choice->GetStringSelection();
			char *t = new char[str_script.Length() + 1];
			strncpy(t, str_script.fn_str(), str_script.Length() + 1);
			f.script = t;
		}

		{
			wxTextCtrl *text = m_program_romimage_picker->GetTextCtrl();
			wxString str_rom = text->GetValue();
			if(text->IsEmpty() == true){
				*m_log << "Enter filename to ROM image\n";
				return;
			}
			char *t = new char[str_rom.Length() + 1];
			strncpy(t, str_rom.fn_str(), str_rom.Length() + 1);
			f.target = t;
		}
		f.compare = m_program_compare->GetValue();
		f.testrun = false;

		if(program_rom_set(
			m_program_cpu_device->GetStringSelection(), 
			m_program_cpu_padding->GetSelection(),
			&f.cpu.memory, &f.cpu.flash
		) == false){
			return;
		}
		if(program_rom_set(
			m_program_ppu_device->GetStringSelection(), 
			m_program_ppu_padding->GetSelection(),
			&f.ppu.memory, &f.ppu.flash
		) == false){
			return;
		}

		f.control = &DRIVER_KAZZO.control;
		f.cpu.access = &DRIVER_KAZZO.cpu;
		f.ppu.access = &DRIVER_KAZZO.ppu;

		m_program_script_choice->Disable();
		m_program_romimage_picker->Disable();
		m_program_compare->Disable();
		m_program_button->SetLabel(wxString("cancel"));
		m_program_cpu_padding->Disable();
		m_program_cpu_device->Disable();
		m_program_ppu_padding->Disable();
		m_program_ppu_device->Disable();
		m_program_compare->Disable();

		m_anago_thread = new anago_programmer(this, &f);
		if(m_anago_thread->Create() != wxTHREAD_NO_ERROR){
			*m_log << "thread creating error";
		}else if(m_anago_thread->Run() != wxTHREAD_NO_ERROR){
			*m_log << "thread running error";
		}else{
			m_status = STATUS_PROGRAMMING;
		}
	}

protected:
	void dump_button_click(wxCommandEvent& event)
	{
		switch(m_status){
		case STATUS_IDLE:
			this->dump_execute();
			break;
		case STATUS_DUMPPING:
			m_anago_thread->Kill();
			this->DumpThreadFinish();
			m_status = STATUS_IDLE;
			break;
		default: //do nothing
			break;
		}
	}
	void program_button_click(wxCommandEvent& event)
	{
		switch(m_status){
		case STATUS_IDLE:
			this->program_execute();
			break;
		case STATUS_PROGRAMMING:
			m_anago_thread->Kill();
			this->ProgramThreadFinish();
			m_status = STATUS_IDLE;
			break;
		default: //do nothing
			break;
		}
	}
	void mapper_change_check(wxCommandEvent& event)
	{
		if(m_dump_check_forcemapper->GetValue() == true){
			m_dump_text_forcemapper->Enable();
		}else{
			m_dump_text_forcemapper->Disable();
		}
	}
	void menu_log_clean(wxCommandEvent& event)
	{
		m_log->Clear();
	}
public:
	/** Constructor */
	anago_frame( wxWindow* parent ) : frame_main(parent)
	{
		this->script_choice_init(m_dump_script_choice, wxString("*.ad"));
		this->script_choice_init(m_program_script_choice, wxString("*.af"));
		this->dump_increase_init(m_dump_cpu_increase);
		this->dump_increase_init(m_dump_ppu_increase);

		struct flash_listup list;
		list.obj_cpu = m_program_cpu_device;
		list.obj_ppu = m_program_ppu_device;
		list.append = choice_append;
		flash_device_listup(&list);
		if(m_program_cpu_device->GetCount() == 0){
			*m_log << "warning: flash device parameter not found\n";
		}else{
			m_program_cpu_device->Select(0);
			m_program_ppu_device->Select(0);
		}
		this->program_padding_init(m_program_cpu_padding);
		this->program_padding_init(m_program_ppu_padding);
		
		m_anago_thread = NULL;
		m_status = STATUS_IDLE;
	}

	void DumpThreadFinish(void)
	{
		m_dump_script_choice->Enable();
		m_dump_romimage_picker->Enable();
		m_dump_check_battery->Enable();
		m_dump_check_forcemapper->Enable();
		m_dump_cpu_increase->Enable();
		m_dump_ppu_increase->Enable();
		m_dump_button->SetLabel(wxString("&dump"));
		if(m_dump_check_forcemapper->GetValue() == true){
			m_dump_text_forcemapper->Enable();
		}
		m_status = STATUS_IDLE;
	}
	
	void ProgramThreadFinish(void)
	{
		m_program_script_choice->Enable();
		m_program_romimage_picker->Enable();
		m_program_compare->Enable();
		m_program_button->SetLabel(wxString("&program"));
		m_program_cpu_padding->Enable();
		m_program_cpu_device->Enable();
		m_program_ppu_padding->Enable();
		m_program_ppu_device->Enable();
		m_status = STATUS_IDLE;
	}
};


void *anago_dumper::Entry(void)
{
	script_dump_execute(&m_config);
	m_frame->DumpThreadFinish();
	return NULL;
}

void *anago_programmer::Entry(void)
{
	script_program_execute(&m_config);
	m_frame->ProgramThreadFinish();
	return NULL;
}

class MyApp : public wxApp
{
private:
	anago_frame *m_frame;
public: 
	bool OnInit()
	{
		m_frame = new anago_frame(NULL);
		m_frame->Show();
		
		return true;
	}
};
IMPLEMENT_APP(MyApp)
