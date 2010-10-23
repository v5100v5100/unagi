///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __anago_gui__
#define __anago_gui__

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/gauge.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/notebook.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class frame_main
///////////////////////////////////////////////////////////////////////////////
class frame_main : public wxFrame 
{
	private:
	
	protected:
		wxNotebook* m_notebook3;
		wxPanel* m_panel_dump;
		wxStaticText* m_dump_script_label;
		wxChoice* m_dump_script_choice;
		wxStaticText* m_dump_romimage_label;
		wxFilePickerCtrl* m_dump_romimage_picker;
		wxStaticLine* m_staticline1;
		wxStaticText* m_dump_cpu_label;
		wxGauge* m_dump_cpu_gauge;
		wxChoice* m_dump_cpu_increase;
		wxStaticText* m_dump_cpu_value;
		wxStaticLine* m_staticline2;
		wxStaticText* m_dump_ppu_label;
		wxGauge* m_dump_ppu_gauge;
		wxChoice* m_dump_ppu_increase;
		wxStaticText* m_dump_ppu_value;
		wxStaticLine* m_staticline3;
		wxCheckBox* m_dump_check_battery;
		wxCheckBox* m_dump_check_forcemapper;
		wxTextCtrl* m_dump_text_forcemapper;
		wxButton* m_dump_button;
		wxPanel* m_panel_program;
		wxStaticText* m_program_script_label;
		wxChoice* m_program_script_choice;
		wxStaticText* m_program_label;
		wxFilePickerCtrl* m_program_romimage_picker;
		wxStaticLine* m_staticline11;
		wxStaticText* m_program_cpu_label;
		wxGauge* m_program_cpu_gauge;
		wxChoice* m_program_cpu_device;
		wxChoice* m_program_cpu_padding;
		wxStaticText* m_program_cpu_value;
		wxStaticLine* m_staticline21;
		wxStaticText* m_program_ppu_label;
		wxGauge* m_program_ppu_gauge;
		wxChoice* m_program_ppu_device;
		wxChoice* m_program_ppu_padding;
		wxStaticText* m_program_ppu_value;
		wxStaticLine* m_staticline31;
		wxCheckBox* m_program_compare;
		wxButton* m_program_button;
		wxPanel* m_panel4;
		wxStaticBitmap* m_bitmap1;
		wxPanel* m_panel_log;
		wxTextCtrl* m_log;
		
		// Virtual event handlers, overide them in your derived class
		virtual void mapper_change_check( wxCommandEvent& event ) { event.Skip(); }
		virtual void dump_button_click( wxCommandEvent& event ) { event.Skip(); }
		virtual void program_button_click( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		frame_main( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("anago wx"), const wxPoint& pos = wxPoint( 32,32 ), const wxSize& size = wxSize( 340,460 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		~frame_main();
	
};

#endif //__anago_gui__
