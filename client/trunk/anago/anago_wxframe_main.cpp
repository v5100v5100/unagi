#include <wx/wx.h>
#include <wx/log.h>
#include <wx/dir.h>
#include <wx/thread.h>
#include <wx/app.h>
#include "wx.h"
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

static void choice_append(void *choice, const char *str)
{
	wxChoice *c = static_cast<wxChoice *>(choice);
	c->Append(wxString(str));
}

class anago_wxframe_main;
class anago_execute : public wxThread
{
private:
	anago_wxframe_main *m_frame;
	struct config_dump m_config;
protected:
	virtual void *Entry(void);
	virtual void OnExit(void)
	{
	}
public:
	anago_execute(anago_wxframe_main *f, struct config_dump *d) : wxThread()
	{
		m_frame = f;
		memcpy(&m_config, d, sizeof(config_dump));
	}
};


class anago_wxframe_main : public frame_main
{
private:
	enum {
		MODE_DUMP, MODE_PROGRAM
	} m_mode;
	anago_execute *m_thread_dump;
	
	void choise_script_init(wxString filespec)
	{
		wxDir dir(wxGetCwd());
		wxString filename;

		m_choice_script->Clear();
		if ( !dir.IsOpened() ){
			return;
		}
		bool cont = dir.GetFirst(&filename, filespec, wxDIR_FILES);
		while ( cont ){
			m_choice_script->Append(filename);
			cont = dir.GetNext(&filename);
		}
		m_choice_script->Select(0);
	}
	void dump_form_init(void)
	{
		m_mode = MODE_DUMP;
		this->choise_script_init(wxString("*.ad"));
//		m_picker_romimage->GetTextCtrl()->Clear();
		m_check_battery->Show();
		m_check_forcemapper->Show();
		m_text_forcemapper->Show();
		m_check_compare->Hide();
		m_button_execute->SetLabel(wxString("&Dump"));
		m_choice_cpu_trans->Hide();
		m_choice_cpu_device->Hide();
		m_choice_ppu_trans->Hide();
		m_choice_ppu_device->Hide();
	}
	void choise_trans_init(wxChoice *c)
	{
		c->Clear();
		c->Append(wxString("full"));
		c->Append(wxString("top"));
		c->Append(wxString("bottom"));
		c->Append(wxString("empty"));
		c->Select(0);
	}
	void program_form_init(void)
	{
		m_mode = MODE_PROGRAM;
		this->choise_script_init(wxString("*.af"));
//		m_picker_romimage->GetTextCtrl()->Clear();
		m_check_battery->Hide();
		m_check_forcemapper->Hide();
		m_text_forcemapper->Hide();
		m_check_compare->Show();
		m_button_execute->SetLabel(wxString("&Program"));
		m_choice_cpu_trans->Show();
		m_choice_cpu_device->Show();
		m_choice_ppu_trans->Show();
		m_choice_ppu_device->Show();
	}
	void gauge_init(struct gauge *t)
	{
		t->label_set = label_set;
		t->range_set = range_set;
		t->value_set = value_set;
	}
	void dump_execute(void)
	{
		struct config_dump config;
		config.gauge_cpu.bar = m_gauge_cpu;
		config.gauge_cpu.label = m_label_cpu;
		gauge_init(&config.gauge_cpu);

		config.gauge_ppu.bar = m_gauge_ppu;
		config.gauge_ppu.label = m_label_ppu;
		gauge_init(&config.gauge_ppu);
		
		config.log.object = m_log;
		config.log.append = text_append;
		config.increase.cpu = 1;
		config.increase.ppu = 1;
		config.progress = true;
		wxString str_script = m_choice_script->GetStringSelection();
		strncpy(config.script, str_script.fn_str(), SCRIPT_STR_LENGTH);

		{
			wxString str;
			config.mappernum = -1;
			if(m_check_forcemapper->GetValue() == true){
				str = m_text_forcemapper->GetValue();
				if(str.ToLong(&config.mappernum) == false){
					*m_log << "bad mapper number\n";
					return;
				}
			}
		}

		wxTextCtrl *text = m_picker_romimage->GetTextCtrl();
		wxString str_rom = text->GetValue();
		if(text->IsEmpty() == true){
			*m_log << "Enter filename to ROM image\n";
			return;
		}
		strncpy(config.target, str_rom.fn_str(), TARGET_STR_LENGTH);

		config.reader = &DRIVER_KAZZO;
		if(config.reader->open_or_close(READER_OPEN) == NG){
			*m_log << "reader open error\n";
			return;
		}
		m_choice_script->Disable();
		m_picker_romimage->Disable();
		m_check_battery->Disable();
		m_check_forcemapper->Disable();
		m_button_execute->Disable();
		if(m_check_forcemapper->GetValue() == true){
			m_text_forcemapper->Disable();
		}

		config.reader->init();
/*		script_dump_execute(&config);
		config.reader->open_or_close(READER_CLOSE);*/
		m_thread_dump = new anago_execute(this, &config);
		if(m_thread_dump->Create() != wxTHREAD_NO_ERROR){
			*m_log << "thread creating error";
		}else if(m_thread_dump->Run() != wxTHREAD_NO_ERROR){
			*m_log << "thread running error";
		}
	}
	
	bool rom_set(const char *area, wxString device, int trans, struct memory *m, struct flash_device *f)
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
		
		f.gauge_cpu.bar = m_gauge_cpu;
		f.gauge_cpu.label = m_label_cpu;
		gauge_init(&f.gauge_cpu);

		f.gauge_ppu.bar = m_gauge_ppu;
		f.gauge_ppu.label = m_label_ppu;
		gauge_init(&f.gauge_ppu);
		
		f.log.object = m_log;
		f.log.append = text_append;
		
		wxString str_script = m_choice_script->GetStringSelection();
		f.script = str_script.fn_str();

		wxTextCtrl *text = m_picker_romimage->GetTextCtrl();
		wxString str_rom = text->GetValue();
		if(text->IsEmpty() == true){
			*m_log << "Enter filename to ROM image\n";
			return;
		}
		f.target = str_rom.fn_str();
		f.compare = false;
		f.testrun = false;
		if(nesfile_load(__FUNCTION__, f.target, &f.rom) == false){
			return;
		}
		if(rom_set(
			"CPU", m_choice_cpu_device->GetStringSelection(), 
			m_choice_cpu_trans->GetSelection(),
			&f.rom.cpu_rom, &f.flash_cpu
		) == false){
			return;
		}
		if(rom_set(
			"PPU", m_choice_ppu_device->GetStringSelection(), 
			m_choice_ppu_trans->GetSelection(),
			&f.rom.ppu_rom, &f.flash_ppu
		) == false){
			return;
		}

		f.reader = &DRIVER_KAZZO;
		if(f.reader->open_or_close(READER_OPEN) == NG){
			*m_log << "reader open error\n";
			return;
		}

		m_choice_script->Disable();
		m_picker_romimage->Disable();
		m_check_compare->Disable();
		m_button_execute->Disable();
		m_choice_cpu_trans->Disable();
		m_choice_cpu_device->Disable();
		m_choice_ppu_trans->Disable();
		m_choice_ppu_device->Disable();
		f.reader->init();
		script_flash_execute(&f);

		nesbuffer_free(&f.rom, 0);
		f.reader->open_or_close(READER_CLOSE);

		m_choice_script->Enable();
		m_picker_romimage->Enable();
		m_check_compare->Enable();
		m_button_execute->Enable();
		m_choice_cpu_trans->Enable();
		m_choice_cpu_device->Enable();
		m_choice_ppu_trans->Enable();
		m_choice_ppu_device->Enable();
	}
protected:
	// Handlers for frame_main events.
	void OnButtonClick( wxCommandEvent& event )
	{
		switch(m_mode){
		case MODE_DUMP:
			this->dump_execute();
			break;
		case MODE_PROGRAM:
			this->program_execute();
			break;
		}
	}

	void mapper_change_check(wxCommandEvent& event)
	{
		if(m_check_forcemapper->GetValue() == true){
			m_text_forcemapper->Enable();
		}else{
			m_text_forcemapper->Disable();
		}
	}
	void menu_log_clean(wxCommandEvent& event)
	{
		m_log->Clear();
	}
	void menu_exit(wxCommandEvent& event)
	{
		event.Skip();
	}
	void menu_mode_dump_set(wxCommandEvent& event)
	{
		if(m_mode != MODE_DUMP){
			this->dump_form_init();
		}
	}
	void menu_mode_program_set(wxCommandEvent& event)
	{
		if(m_mode != MODE_PROGRAM){
			this->program_form_init();
		}
	}
	void menu_help_contents(wxCommandEvent& event)
	{
		*m_log << wxString("たすケて\n");
	}
	
public:
	/** Constructor */
	anago_wxframe_main( wxWindow* parent ) : frame_main (parent)
	{
		this->dump_form_init();
		//this->program_form_init();
		this->choise_trans_init(m_choice_cpu_trans);
		this->choise_trans_init(m_choice_ppu_trans);

		struct flash_listup list;
		list.obj_cpu = m_choice_cpu_device;
		list.obj_ppu = m_choice_ppu_device;
		list.append = choice_append;
		flash_device_listup(&list);
		m_choice_cpu_device->Select(0);
		m_choice_ppu_device->Select(0);
	}
	
	void DumpThreadFinish(void)
	{
		m_choice_script->Enable();
		m_picker_romimage->Enable();
		m_check_battery->Enable();
		m_check_forcemapper->Enable();
		m_button_execute->Enable();
		if(m_check_forcemapper->GetValue() == true){
			m_text_forcemapper->Enable();
		}
	}
};

void *anago_execute::Entry(void)
{
	script_dump_execute(&m_config);
	m_config.reader->open_or_close(READER_CLOSE);
	m_frame->DumpThreadFinish();
	return NULL;
}

class MyApp : public wxApp
{
private:
	anago_wxframe_main *m_frame;
public: 
	bool OnInit()
	{
		m_frame = new anago_wxframe_main(NULL);
		m_frame->Show();
		
		return true;
	}
};
IMPLEMENT_APP(MyApp)
