///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __anago_gui__
#define __anago_gui__

#include <wx/gdicmn.h>
#include <wx/notebook.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/stattext.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbmp.h>
#include <wx/statline.h>
#include <wx/choice.h>
#include <wx/filepicker.h>
#include <wx/gauge.h>
#include <wx/button.h>
#include <wx/checkbox.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class frame_main
///////////////////////////////////////////////////////////////////////////////
class frame_main : public wxFrame 
{
	private:
	
	protected:
		wxNotebook* m_notebook;
		wxPanel* m_panel_log;
		wxTextCtrl* m_log;
	
	public:
		
		frame_main( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = wxT("anago wx"), const wxPoint& pos = wxPoint( 32,32 ), const wxSize& size = wxSize( 340,460 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );
		~frame_main();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class panel_version
///////////////////////////////////////////////////////////////////////////////
class panel_version : public wxPanel 
{
	private:
	
	protected:
		wxStaticText* m_version_title;
		wxStaticText* m_version_copyright;
		wxStaticBitmap* m_version_photo;
		wxStaticText* m_version_developer;
		wxTextCtrl* m_version_detail;
	
	public:
		
		panel_version( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL );
		~panel_version();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class panel_workram
///////////////////////////////////////////////////////////////////////////////
class panel_workram : public wxPanel 
{
	private:
	
	protected:
		wxStaticLine* m_staticline111;
		wxStaticText* m_staticText221;
		wxStaticLine* m_staticline71;
		wxStaticText* m_staticText16;
		wxChoice* m_read_script;
		wxStaticText* m_staticText17;
		wxFilePickerCtrl* m_read_picker;
		wxGauge* m_read_gauge;
		wxButton* m_read_button;
		wxStaticLine* m_staticline12;
		wxStaticText* m_staticText22;
		wxStaticLine* m_staticline7;
		wxStaticText* m_staticText161;
		wxChoice* m_write_script;
		wxStaticText* m_staticText171;
		wxFilePickerCtrl* m_write_picker;
		wxGauge* m_write_gauge;
		wxButton* m_write_button;
		
		// Virtual event handlers, overide them in your derived class
		virtual void read_button_click( wxCommandEvent& event ) { event.Skip(); }
		virtual void write_button_click( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		panel_workram( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL );
		~panel_workram();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class panel_dump
///////////////////////////////////////////////////////////////////////////////
class panel_dump : public wxPanel 
{
	private:
	
	protected:
		wxStaticText* m_script_label;
		wxChoice* m_script_choice;
		wxStaticText* m_romimage_label;
		wxFilePickerCtrl* m_romimage_picker;
		wxStaticLine* m_staticline1;
		wxStaticText* m_cpu_label;
		wxGauge* m_cpu_gauge;
		wxChoice* m_cpu_increase;
		wxStaticText* m_cpu_value;
		wxStaticLine* m_staticline2;
		wxStaticText* m_ppu_label;
		wxGauge* m_ppu_gauge;
		wxChoice* m_ppu_increase;
		wxStaticText* m_ppu_value;
		wxStaticLine* m_staticline3;
		wxCheckBox* m_check_battery;
		wxCheckBox* m_check_forcemapper;
		wxTextCtrl* m_text_forcemapper;
		wxButton* m_button;
		
		// Virtual event handlers, overide them in your derived class
		virtual void mapper_change_check( wxCommandEvent& event ) { event.Skip(); }
		virtual void button_click( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		panel_dump( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL );
		~panel_dump();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class panel_program
///////////////////////////////////////////////////////////////////////////////
class panel_program : public wxPanel 
{
	private:
	
	protected:
		wxStaticText* m_script_label;
		wxChoice* m_script_choice;
		wxStaticText* m_romimage_label;
		wxFilePickerCtrl* m_romimage_picker;
		wxStaticLine* m_staticline11;
		wxStaticText* m_cpu_label;
		wxGauge* m_cpu_gauge;
		wxChoice* m_cpu_device;
		wxChoice* m_cpu_padding;
		wxStaticText* m_cpu_value;
		wxStaticLine* m_staticline21;
		wxStaticText* m_ppu_label;
		wxGauge* m_ppu_gauge;
		wxChoice* m_ppu_device;
		wxChoice* m_ppu_padding;
		wxStaticText* m_ppu_value;
		wxStaticLine* m_staticline31;
		wxCheckBox* m_compare;
		wxButton* m_button;
		
		// Virtual event handlers, overide them in your derived class
		virtual void button_click( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		
		panel_program( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL );
		~panel_program();
	
};

#endif //__anago_gui__
