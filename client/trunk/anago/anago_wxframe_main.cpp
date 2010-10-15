#include <stdio.h>
#include <wx/wx.h>
#include <wx/log.h>
#include <wx/dir.h>
#include "widget.h"
#include "anago_wxframe_main.h"
#include "reader_master.h"
//#include "reader_kazzo.h"
extern const struct reader_driver DRIVER_KAZZO;
extern "C"{
#include "script_dump.h"
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

anago_wxframe_main::anago_wxframe_main( wxWindow* parent )
  : frame_main( parent )
{

	wxDir dir(wxGetCwd());
	wxString filename;


	if ( !dir.IsOpened() ){
		return;
	}
	bool cont = dir.GetFirst(&filename, wxString("*.ad"), wxDIR_FILES);
	while ( cont ){
		m_combo_script->Append(filename);

		cont = dir.GetNext(&filename);
	}
	m_combo_script->Select(0);
}

void anago_wxframe_main::OnButtonClick(wxCommandEvent& event)
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
	wxString str_script = m_combo_script->GetValue();
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
	m_combo_script->Disable();
	m_picker_romimage->Disable();
	m_check_battery->Disable();
	m_check_forcemapper->Disable();
	m_button_dump->Disable();
	if(m_check_forcemapper->GetValue() == true){
		m_text_forcemapper->Disable();
	}

	config.reader->init();
	script_dump_execute(&config);
	config.reader->open_or_close(READER_CLOSE);

	m_combo_script->Enable();
	m_picker_romimage->Enable();
	m_check_battery->Enable();
	m_check_forcemapper->Enable();
	m_button_dump->Enable();
	if(m_check_forcemapper->GetValue() == true){
		m_text_forcemapper->Enable();
	}
}

void anago_wxframe_main::mapper_change_check(wxCommandEvent& event)
{
	if(m_check_forcemapper->GetValue() == true){
		m_text_forcemapper->Enable();
	}else{
		m_text_forcemapper->Disable();
	}
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
/*	int OnExit()
	{
		delete m_frame;
		m_frame = NULL;
		
		return 0;
	}*/
};
IMPLEMENT_APP(MyApp)
