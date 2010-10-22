///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Sep  8 2010)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "anago_gui.h"

///////////////////////////////////////////////////////////////////////////

frame_main::frame_main( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );
	
	m_notebook3 = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_panel_dump = new wxPanel( m_notebook3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_dump_script_label = new wxStaticText( m_panel_dump, wxID_ANY, wxT("&script"), wxDefaultPosition, wxDefaultSize, 0 );
	m_dump_script_label->Wrap( -1 );
	fgSizer1->Add( m_dump_script_label, 0, wxALL, 5 );
	
	wxArrayString m_dump_script_choiceChoices;
	m_dump_script_choice = new wxChoice( m_panel_dump, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_dump_script_choiceChoices, 0 );
	m_dump_script_choice->SetSelection( 0 );
	fgSizer1->Add( m_dump_script_choice, 0, wxALL|wxEXPAND, 5 );
	
	m_dump_romimage_label = new wxStaticText( m_panel_dump, wxID_ANY, wxT("&ROM image"), wxDefaultPosition, wxDefaultSize, 0 );
	m_dump_romimage_label->Wrap( -1 );
	fgSizer1->Add( m_dump_romimage_label, 0, wxALL, 5 );
	
	m_dump_romimage_picker = new wxFilePickerCtrl( m_panel_dump, wxID_ANY, wxEmptyString, wxT("Select a file"), wxT("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE|wxFLP_OVERWRITE_PROMPT|wxFLP_SAVE );
	fgSizer1->Add( m_dump_romimage_picker, 0, wxALL|wxEXPAND, 5 );
	
	bSizer9->Add( fgSizer1, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxHORIZONTAL );
	
	m_dump_cpu_label = new wxStaticText( m_panel_dump, wxID_ANY, wxT("CPU"), wxDefaultPosition, wxDefaultSize, 0 );
	m_dump_cpu_label->Wrap( -1 );
	m_dump_cpu_label->SetMinSize( wxSize( 150,-1 ) );
	
	bSizer12->Add( m_dump_cpu_label, 0, wxALL, 5 );
	
	wxArrayString m_dump_cpu_increaseChoices;
	m_dump_cpu_increase = new wxChoice( m_panel_dump, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_dump_cpu_increaseChoices, 0 );
	m_dump_cpu_increase->SetSelection( 0 );
	m_dump_cpu_increase->SetMinSize( wxSize( 60,-1 ) );
	
	bSizer12->Add( m_dump_cpu_increase, 0, wxALL, 5 );
	
	bSizer9->Add( bSizer12, 1, wxEXPAND, 5 );
	
	m_dump_cpu_gauge = new wxGauge( m_panel_dump, wxID_ANY, 100, wxDefaultPosition, wxSize( -1,12 ), wxGA_HORIZONTAL );
	bSizer9->Add( m_dump_cpu_gauge, 0, wxALL|wxEXPAND, 0 );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxHORIZONTAL );
	
	m_dump_ppu_label = new wxStaticText( m_panel_dump, wxID_ANY, wxT("PPU"), wxDefaultPosition, wxDefaultSize, 0 );
	m_dump_ppu_label->Wrap( -1 );
	m_dump_ppu_label->SetMinSize( wxSize( 150,-1 ) );
	
	bSizer13->Add( m_dump_ppu_label, 0, wxALL, 5 );
	
	wxArrayString m_dump_ppu_increaseChoices;
	m_dump_ppu_increase = new wxChoice( m_panel_dump, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_dump_ppu_increaseChoices, 0 );
	m_dump_ppu_increase->SetSelection( 0 );
	m_dump_ppu_increase->SetMinSize( wxSize( 60,-1 ) );
	
	bSizer13->Add( m_dump_ppu_increase, 0, wxALL, 5 );
	
	bSizer9->Add( bSizer13, 1, wxEXPAND, 5 );
	
	m_dump_ppu_gauge = new wxGauge( m_panel_dump, wxID_ANY, 100, wxDefaultPosition, wxSize( -1,12 ), wxGA_HORIZONTAL );
	bSizer9->Add( m_dump_ppu_gauge, 0, wxALL|wxEXPAND, 0 );
	
	wxBoxSizer* bSizer61;
	bSizer61 = new wxBoxSizer( wxHORIZONTAL );
	
	m_dump_check_battery = new wxCheckBox( m_panel_dump, wxID_ANY, wxT("&battery"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer61->Add( m_dump_check_battery, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_dump_check_forcemapper = new wxCheckBox( m_panel_dump, wxID_ANY, wxT("&change mapper"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer61->Add( m_dump_check_forcemapper, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_dump_text_forcemapper = new wxTextCtrl( m_panel_dump, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_dump_text_forcemapper->Enable( false );
	m_dump_text_forcemapper->SetMinSize( wxSize( 40,-1 ) );
	
	bSizer61->Add( m_dump_text_forcemapper, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_dump_button = new wxButton( m_panel_dump, wxID_ANY, wxT("&dump"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer61->Add( m_dump_button, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer9->Add( bSizer61, 0, wxALIGN_RIGHT, 5 );
	
	m_panel_dump->SetSizer( bSizer9 );
	m_panel_dump->Layout();
	bSizer9->Fit( m_panel_dump );
	m_notebook3->AddPage( m_panel_dump, wxT("dump"), false );
	m_panel_program = new wxPanel( m_notebook3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer91;
	bSizer91 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer11;
	fgSizer11 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer11->AddGrowableCol( 1 );
	fgSizer11->SetFlexibleDirection( wxBOTH );
	fgSizer11->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_program_script_label = new wxStaticText( m_panel_program, wxID_ANY, wxT("&script"), wxDefaultPosition, wxDefaultSize, 0 );
	m_program_script_label->Wrap( -1 );
	fgSizer11->Add( m_program_script_label, 0, wxALL, 5 );
	
	wxArrayString m_program_script_choiceChoices;
	m_program_script_choice = new wxChoice( m_panel_program, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_program_script_choiceChoices, 0 );
	m_program_script_choice->SetSelection( 0 );
	fgSizer11->Add( m_program_script_choice, 0, wxALL|wxEXPAND, 5 );
	
	m_program_romimage_label = new wxStaticText( m_panel_program, wxID_ANY, wxT("&ROM image"), wxDefaultPosition, wxDefaultSize, 0 );
	m_program_romimage_label->Wrap( -1 );
	fgSizer11->Add( m_program_romimage_label, 0, wxALL, 5 );
	
	m_program_romimage_picker = new wxFilePickerCtrl( m_panel_program, wxID_ANY, wxEmptyString, wxT("Select a file"), wxT("*.nes"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE|wxFLP_FILE_MUST_EXIST );
	fgSizer11->Add( m_program_romimage_picker, 0, wxALL|wxEXPAND, 5 );
	
	bSizer91->Add( fgSizer11, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer121;
	bSizer121 = new wxBoxSizer( wxHORIZONTAL );
	
	m_program_cpu_label = new wxStaticText( m_panel_program, wxID_ANY, wxT("CPU"), wxDefaultPosition, wxDefaultSize, 0 );
	m_program_cpu_label->Wrap( -1 );
	m_program_cpu_label->SetMinSize( wxSize( 150,-1 ) );
	
	bSizer121->Add( m_program_cpu_label, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer31;
	bSizer31 = new wxBoxSizer( wxHORIZONTAL );
	
	wxArrayString m_program_cpu_deviceChoices;
	m_program_cpu_device = new wxChoice( m_panel_program, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_program_cpu_deviceChoices, 0 );
	m_program_cpu_device->SetSelection( 0 );
	m_program_cpu_device->SetMinSize( wxSize( 80,-1 ) );
	
	bSizer31->Add( m_program_cpu_device, 0, wxALL, 5 );
	
	wxArrayString m_program_cpu_paddingChoices;
	m_program_cpu_padding = new wxChoice( m_panel_program, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_program_cpu_paddingChoices, 0 );
	m_program_cpu_padding->SetSelection( 0 );
	m_program_cpu_padding->SetMinSize( wxSize( 60,-1 ) );
	
	bSizer31->Add( m_program_cpu_padding, 0, wxALL, 5 );
	
	bSizer121->Add( bSizer31, 1, wxEXPAND, 5 );
	
	bSizer91->Add( bSizer121, 1, wxEXPAND, 5 );
	
	m_program_cpu_gauge = new wxGauge( m_panel_program, wxID_ANY, 100, wxDefaultPosition, wxSize( -1,12 ), wxGA_HORIZONTAL );
	bSizer91->Add( m_program_cpu_gauge, 0, wxALL|wxEXPAND, 0 );
	
	wxBoxSizer* bSizer131;
	bSizer131 = new wxBoxSizer( wxHORIZONTAL );
	
	m_program_ppu_label = new wxStaticText( m_panel_program, wxID_ANY, wxT("PPU"), wxDefaultPosition, wxDefaultSize, 0 );
	m_program_ppu_label->Wrap( -1 );
	m_program_ppu_label->SetMinSize( wxSize( 150,-1 ) );
	
	bSizer131->Add( m_program_ppu_label, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer( wxHORIZONTAL );
	
	wxArrayString m_program_ppu_deviceChoices;
	m_program_ppu_device = new wxChoice( m_panel_program, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), m_program_ppu_deviceChoices, 0 );
	m_program_ppu_device->SetSelection( 0 );
	m_program_ppu_device->SetMinSize( wxSize( 80,-1 ) );
	
	bSizer30->Add( m_program_ppu_device, 0, wxALL, 5 );
	
	wxArrayString m_program_ppu_paddingChoices;
	m_program_ppu_padding = new wxChoice( m_panel_program, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_program_ppu_paddingChoices, 0 );
	m_program_ppu_padding->SetSelection( 0 );
	m_program_ppu_padding->SetMinSize( wxSize( 60,-1 ) );
	
	bSizer30->Add( m_program_ppu_padding, 0, wxALL, 5 );
	
	bSizer131->Add( bSizer30, 0, 0, 5 );
	
	bSizer91->Add( bSizer131, 1, wxALIGN_RIGHT|wxEXPAND, 5 );
	
	m_program_ppu_gauge = new wxGauge( m_panel_program, wxID_ANY, 100, wxDefaultPosition, wxSize( -1,12 ), wxGA_HORIZONTAL );
	bSizer91->Add( m_program_ppu_gauge, 0, wxALL|wxEXPAND, 0 );
	
	wxBoxSizer* bSizer611;
	bSizer611 = new wxBoxSizer( wxHORIZONTAL );
	
	m_program_compare = new wxCheckBox( m_panel_program, wxID_ANY, wxT("compare"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer611->Add( m_program_compare, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_program_button = new wxButton( m_panel_program, wxID_ANY, wxT("&program"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer611->Add( m_program_button, 0, wxALL|wxALIGN_BOTTOM, 5 );
	
	bSizer91->Add( bSizer611, 0, wxALIGN_RIGHT, 5 );
	
	m_panel_program->SetSizer( bSizer91 );
	m_panel_program->Layout();
	bSizer91->Fit( m_panel_program );
	m_notebook3->AddPage( m_panel_program, wxT("program"), false );
	
	bSizer4->Add( m_notebook3, 0, wxALL|wxEXPAND, 0 );
	
	m_panel_log = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );
	
	m_log = new wxTextCtrl( m_panel_log, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	bSizer6->Add( m_log, 1, wxALL|wxEXPAND, 5 );
	
	m_panel_log->SetSizer( bSizer6 );
	m_panel_log->Layout();
	bSizer6->Fit( m_panel_log );
	bSizer4->Add( m_panel_log, 1, wxALL|wxEXPAND, 0 );
	
	this->SetSizer( bSizer4 );
	this->Layout();
	
	// Connect Events
	m_dump_check_forcemapper->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( frame_main::mapper_change_check ), NULL, this );
	m_dump_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( frame_main::dump_button_click ), NULL, this );
	m_program_button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( frame_main::program_button_click ), NULL, this );
}

frame_main::~frame_main()
{
	// Disconnect Events
	m_dump_check_forcemapper->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( frame_main::mapper_change_check ), NULL, this );
	m_dump_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( frame_main::dump_button_click ), NULL, this );
	m_program_button->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( frame_main::program_button_click ), NULL, this );
	
}
