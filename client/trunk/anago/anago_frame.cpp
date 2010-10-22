#include <wx/wx.h>
#include <wx/log.h>
#include <wx/dir.h>
#include <wx/thread.h>
#include <wx/app.h>
#include <cstring>
#include "anago_gui.h"
#include "widget.h"
#include "reader_master.h"
//#include "reader_kazzo.h"
extern const struct reader_driver DRIVER_KAZZO;
extern "C"{
#include "header.h"
#include "flash_device.h"
#include "script_dump.h"
#include "script_flash.h"
}

static void value_set(void *gauge, int value)
{
	wxGauge *g = static_cast<wxGauge *>(gauge);
	wxMutexGuiEnter();
	g->SetValue(value);
	wxMutexGuiLeave();
}

static void range_set(void *gauge, int value)
{
	wxGauge *g = static_cast<wxGauge *>(gauge);
	if(value == 0){
		value = 1;
	}
	g->SetRange(value);
}

static void text_append(void *log, const char *str)
{
	wxTextCtrl *l = static_cast<wxTextCtrl *>(log);
	wxMutexGuiEnter();
	*l << str;
	wxMutexGuiLeave();
}

static void label_set(void *label, const char *str)
{
	wxStaticText *l = static_cast<wxStaticText *>(label);
	wxMutexGuiEnter();
	l->SetLabel(str);
	wxMutexGuiLeave();
}

void choice_append(void *choice, const char *str)
{
	wxChoice *c = static_cast<wxChoice *>(choice);
	c->Append(wxString(str));
}

class anago_frame;
class anago_dumper : public wxThread
{
private:
	anago_frame *m_frame;
	struct config_dump m_config;
protected:
	virtual void *Entry(void);
public:
	anago_dumper(anago_frame *f, struct config_dump *d) : wxThread()
	{
		m_frame = f;
		memcpy(&m_config, d, sizeof(config_dump));
	}
};

class anago_programmer : public wxThread
{
private:
	anago_frame *m_frame;
	struct config_flash m_config;
protected:
	virtual void *Entry(void);
public:
	anago_programmer(anago_frame *f, struct config_flash *d) : wxThread()
	{
		m_frame = f;
		memcpy(&m_config, d, sizeof(config_flash));
	}
};

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
	void dump_execute(void)
	{
		struct config_dump config;
		config.gauge_cpu.bar = m_dump_cpu_gauge;
		config.gauge_cpu.label = m_dump_cpu_label;
		gauge_init(&config.gauge_cpu);

		config.gauge_ppu.bar = m_dump_ppu_gauge;
		config.gauge_ppu.label = m_dump_ppu_label;
		gauge_init(&config.gauge_ppu);
		
		config.log.object = m_log;
		config.log.append = text_append;
		config.increase.cpu = 1;
		config.increase.ppu = 1;
		config.progress = true;
		wxString str_script = m_dump_script_choice->GetStringSelection();
		strncpy(config.script, str_script.fn_str(), DUMP_SCRIPT_STR_LENGTH);

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

		wxTextCtrl *text = m_dump_romimage_picker->GetTextCtrl();
		wxString str_rom = text->GetValue();
		if(text->IsEmpty() == true){
			*m_log << "Enter filename to ROM image\n";
			return;
		}
		strncpy(config.target, str_rom.fn_str(), DUMP_TARGET_STR_LENGTH);

		config.reader = &DRIVER_KAZZO;
		if(config.reader->open_or_close(READER_OPEN) == NG){
			*m_log << "reader open error\n";
			return;
		}
		m_dump_script_choice->Disable();
		m_dump_romimage_picker->Disable();
		m_dump_check_battery->Disable();
		m_dump_check_forcemapper->Disable();
		m_dump_button->SetLabel(wxString("cancel"));
		m_dump_text_forcemapper->Disable();
		m_dump_cpu_increase->Disable();
		m_dump_ppu_increase->Disable();

		config.reader->init();
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
	bool program_rom_set(const char *area, wxString device, int trans, struct memory *m, struct flash_device *f)
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
		if(m->size == 0){
			m->transtype = TRANSTYPE_EMPTY;
		}
		if(f->capacity < m->size){
			*m_log << area << "area ROM image size is larger than target device";
			return false;
		}
		return true;
	}
	void program_execute(void)
	{
		struct config_flash f;
		
		f.gauge_cpu.bar = m_program_cpu_gauge;
		f.gauge_cpu.label = m_program_cpu_label;
		gauge_init(&f.gauge_cpu);

		f.gauge_ppu.bar = m_program_ppu_gauge;
		f.gauge_ppu.label = m_program_ppu_label;
		gauge_init(&f.gauge_ppu);
		
		f.log.object = m_log;
		f.log.append = text_append;
		
		wxString str_script = m_program_script_choice->GetStringSelection();
		strncpy(f.script, str_script.fn_str(), PROGRAM_SCRIPT_STR_LENGTH);

		wxTextCtrl *text = m_program_romimage_picker->GetTextCtrl();
		wxString str_rom = text->GetValue();
		if(text->IsEmpty() == true){
			*m_log << "Enter filename to ROM image\n";
			return;
		}
		strncpy(f.target, str_rom.fn_str(), PROGRAM_TARGET_STR_LENGTH);
		f.compare = false;
		f.testrun = false;
//あとで struct config_flash の構造を見直す
//		if(nesfile_load(__FUNCTION__, f.target, &f.rom) == false){
		if(nesfile_load(__FUNCTION__, str_rom.fn_str(), &f.rom) == false){
			*m_log << str_rom << " open error\n";
			return;
		}
		if(program_rom_set(
			"CPU", m_program_cpu_device->GetStringSelection(), 
			m_program_cpu_padding->GetSelection(),
			&f.rom.cpu_rom, &f.flash_cpu
		) == false){
			return;
		}
		if(program_rom_set(
			"PPU", m_program_ppu_device->GetStringSelection(), 
			m_program_ppu_padding->GetSelection(),
			&f.rom.ppu_rom, &f.flash_ppu
		) == false){
			return;
		}

		f.reader = &DRIVER_KAZZO;
		if(f.reader->open_or_close(READER_OPEN) == NG){
			*m_log << "reader open error\n";
			return;
		}

		m_program_script_choice->Disable();
		m_program_romimage_picker->Disable();
		m_program_compare->Disable();
		m_program_button->SetLabel(wxString("cancel"));
		m_program_cpu_padding->Disable();
		m_program_cpu_device->Disable();
		m_program_ppu_padding->Disable();
		m_program_ppu_device->Disable();
		m_program_compare->Disable();
		f.reader->init();
/*		if(m_anago_thread != NULL){
			delete m_anago_thread;
		}*/
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
	m_config.reader->open_or_close(READER_CLOSE);
	m_frame->DumpThreadFinish();
	return NULL;
}

void *anago_programmer::Entry(void)
{
	script_flash_execute(&m_config);
	m_config.reader->open_or_close(READER_CLOSE);
	nesbuffer_free(&m_config.rom, 0);
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
