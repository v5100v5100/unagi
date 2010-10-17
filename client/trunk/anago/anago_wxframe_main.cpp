#include <wx/wx.h>
#include <wx/log.h>
#include <wx/dir.h>
#include "wx.h"
#include "widget.h"
#include "anago_wxframe_main.h"
#include "reader_master.h"
//#include "reader_kazzo.h"
extern const struct reader_driver DRIVER_KAZZO;
extern "C"{
#include "script_dump.h"
#include "flash_device.h"
}

static void value_set(void *gauge, int value)
{
	wxGauge *g = static_cast<wxGauge *>(gauge);
	g->SetValue(value);
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
	*l << str;
}

static void label_set(void *label, const char *str)
{
	wxStaticText *l = static_cast<wxStaticText *>(label);
	l->SetLabel(str);
}

static void choice_append(void *choice, const char *str)
{
	wxChoice *c = static_cast<wxChoice *>(choice);
	c->Append(wxString(str));
}

class anago_wxframe_main : public frame_main
{
private:
	enum {
		MODE_DUMP, MODE_PROGRAM
	} m_mode;
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
		m_check_battery->Show();
		m_check_forcemapper->Show();
		m_text_forcemapper->Show();
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
		c->Append(wxString("empty"));
		c->Append(wxString("top"));
		c->Append(wxString("bottom"));
		c->Select(0);
	}
	void program_form_init(void)
	{
		m_mode = MODE_PROGRAM;
		this->choise_script_init(wxString("*.af"));
		m_check_battery->Hide();
		m_check_forcemapper->Hide();
		m_text_forcemapper->Hide();
		m_button_execute->SetLabel(wxString("&Program"));
		m_choice_cpu_trans->Show();
		m_choice_cpu_device->Show();
		m_choice_ppu_trans->Show();
		m_choice_ppu_device->Show();
	}
	void dump_execute(void)
	{
		struct config_dump config;
		config.gauge_cpu.bar = m_gauge_cpu;
		config.gauge_cpu.label = m_label_cpu;
		config.gauge_cpu.label_set = label_set;
		config.gauge_cpu.range_set = range_set;
		config.gauge_cpu.value_set = value_set;

		config.gauge_ppu.bar = m_gauge_ppu;
		config.gauge_ppu.label = m_label_ppu;
		config.gauge_ppu.label_set = label_set;
		config.gauge_ppu.range_set = range_set;
		config.gauge_ppu.value_set = value_set;
		
		config.log.object = m_log;
		config.log.append = text_append;
		config.increase.cpu = 1;
		config.increase.ppu = 1;
		config.progress = true;
		wxString str_script = m_choice_script->GetStringSelection();
		config.script = str_script.fn_str();

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
		config.target = str_rom.fn_str();

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
		script_dump_execute(&config);
		config.reader->open_or_close(READER_CLOSE);

		m_choice_script->Enable();
		m_picker_romimage->Enable();
		m_check_battery->Enable();
		m_check_forcemapper->Enable();
		m_button_execute->Enable();
		if(m_check_forcemapper->GetValue() == true){
			m_text_forcemapper->Enable();
		}
	}
protected:
	// Handlers for frame_main events.
	void OnButtonClick( wxCommandEvent& event )
	{
		this->dump_execute();
	}

	void mapper_change_check(wxCommandEvent& event)
	{
		if(m_check_forcemapper->GetValue() == true){
			m_text_forcemapper->Enable();
		}else{
			m_text_forcemapper->Disable();
		}
	}
public:
	/** Constructor */
	anago_wxframe_main( wxWindow* parent ) : frame_main (parent)
	{
		//this->dump_form_init();
		this->program_form_init();
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
	
};



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
