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
	this->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	
	wxBoxSizer* bSizer4;
	bSizer4 = new wxBoxSizer( wxVERTICAL );
	
	m_notebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_panel_dump = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
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
	
	m_dump_romimage_picker = new wxFilePickerCtrl( m_panel_dump, wxID_ANY, wxEmptyString, wxT("Select a file"), wxT("*.nes"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE|wxFLP_OVERWRITE_PROMPT|wxFLP_SAVE|wxFLP_USE_TEXTCTRL );
	fgSizer1->Add( m_dump_romimage_picker, 0, wxALL|wxEXPAND, 5 );
	
	bSizer9->Add( fgSizer1, 0, wxEXPAND, 5 );
	
	m_staticline1 = new wxStaticLine( m_panel_dump, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer9->Add( m_staticline1, 0, wxALL|wxEXPAND, 2 );
	
	wxBoxSizer* bSizer12;
	bSizer12 = new wxBoxSizer( wxHORIZONTAL );
	
	m_dump_cpu_label = new wxStaticText( m_panel_dump, wxID_ANY, wxT("Program ROM"), wxDefaultPosition, wxDefaultSize, 0 );
	m_dump_cpu_label->Wrap( -1 );
	m_dump_cpu_label->SetMinSize( wxSize( 80,-1 ) );
	
	bSizer12->Add( m_dump_cpu_label, 0, wxALL, 5 );
	
	m_dump_cpu_gauge = new wxGauge( m_panel_dump, wxID_ANY, 100, wxDefaultPosition, wxSize( -1,12 ), wxGA_HORIZONTAL );
	bSizer12->Add( m_dump_cpu_gauge, 1, wxALL|wxALIGN_CENTER_VERTICAL, 0 );
	
	bSizer9->Add( bSizer12, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer14;
	bSizer14 = new wxBoxSizer( wxHORIZONTAL );
	
	wxArrayString m_dump_cpu_increaseChoices;
	m_dump_cpu_increase = new wxChoice( m_panel_dump, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_dump_cpu_increaseChoices, 0 );
	m_dump_cpu_increase->SetSelection( 0 );
	m_dump_cpu_increase->SetMinSize( wxSize( 60,-1 ) );
	
	bSizer14->Add( m_dump_cpu_increase, 0, wxALL, 5 );
	
	m_dump_cpu_value = new wxStaticText( m_panel_dump, wxID_ANY, wxT("0x000000/0x000000"), wxDefaultPosition, wxDefaultSize, 0 );
	m_dump_cpu_value->Wrap( -1 );
	m_dump_cpu_value->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ) );
	
	bSizer14->Add( m_dump_cpu_value, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer9->Add( bSizer14, 1, wxALIGN_RIGHT, 5 );
	
	m_staticline2 = new wxStaticLine( m_panel_dump, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer9->Add( m_staticline2, 0, wxEXPAND | wxALL, 2 );
	
	wxBoxSizer* bSizer13;
	bSizer13 = new wxBoxSizer( wxHORIZONTAL );
	
	m_dump_ppu_label = new wxStaticText( m_panel_dump, wxID_ANY, wxT("Charcter ROM"), wxDefaultPosition, wxDefaultSize, 0 );
	m_dump_ppu_label->Wrap( -1 );
	m_dump_ppu_label->SetMinSize( wxSize( 80,-1 ) );
	
	bSizer13->Add( m_dump_ppu_label, 0, wxALL, 5 );
	
	m_dump_ppu_gauge = new wxGauge( m_panel_dump, wxID_ANY, 100, wxDefaultPosition, wxSize( -1,12 ), wxGA_HORIZONTAL );
	bSizer13->Add( m_dump_ppu_gauge, 1, wxALL|wxALIGN_CENTER_VERTICAL, 0 );
	
	bSizer9->Add( bSizer13, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer132;
	bSizer132 = new wxBoxSizer( wxHORIZONTAL );
	
	wxArrayString m_dump_ppu_increaseChoices;
	m_dump_ppu_increase = new wxChoice( m_panel_dump, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_dump_ppu_increaseChoices, 0 );
	m_dump_ppu_increase->SetSelection( 0 );
	m_dump_ppu_increase->SetMinSize( wxSize( 60,-1 ) );
	
	bSizer132->Add( m_dump_ppu_increase, 0, wxALL, 5 );
	
	m_dump_ppu_value = new wxStaticText( m_panel_dump, wxID_ANY, wxT("0x000000/0x000000"), wxDefaultPosition, wxDefaultSize, 0 );
	m_dump_ppu_value->Wrap( -1 );
	m_dump_ppu_value->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ) );
	
	bSizer132->Add( m_dump_ppu_value, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer9->Add( bSizer132, 1, wxALIGN_RIGHT, 5 );
	
	m_staticline3 = new wxStaticLine( m_panel_dump, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer9->Add( m_staticline3, 0, wxEXPAND | wxALL, 2 );
	
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
	m_notebook->AddPage( m_panel_dump, wxT("dump"), true );
	m_panel_program = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
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
	
	m_program_label = new wxStaticText( m_panel_program, wxID_ANY, wxT("&ROM image"), wxDefaultPosition, wxDefaultSize, 0 );
	m_program_label->Wrap( -1 );
	fgSizer11->Add( m_program_label, 0, wxALL, 5 );
	
	m_program_romimage_picker = new wxFilePickerCtrl( m_panel_program, wxID_ANY, wxEmptyString, wxT("Select a file"), wxT("*.nes"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE|wxFLP_FILE_MUST_EXIST|wxFLP_USE_TEXTCTRL );
	fgSizer11->Add( m_program_romimage_picker, 0, wxALL|wxEXPAND, 5 );
	
	bSizer91->Add( fgSizer11, 0, wxEXPAND, 5 );
	
	m_staticline11 = new wxStaticLine( m_panel_program, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer91->Add( m_staticline11, 0, wxALL|wxEXPAND, 2 );
	
	wxBoxSizer* bSizer121;
	bSizer121 = new wxBoxSizer( wxHORIZONTAL );
	
	m_program_cpu_label = new wxStaticText( m_panel_program, wxID_ANY, wxT("Program flash"), wxDefaultPosition, wxDefaultSize, 0 );
	m_program_cpu_label->Wrap( -1 );
	m_program_cpu_label->SetMinSize( wxSize( 80,-1 ) );
	
	bSizer121->Add( m_program_cpu_label, 0, wxALL, 5 );
	
	m_program_cpu_gauge = new wxGauge( m_panel_program, wxID_ANY, 100, wxDefaultPosition, wxSize( -1,12 ), wxGA_HORIZONTAL );
	bSizer121->Add( m_program_cpu_gauge, 1, wxALL|wxALIGN_CENTER_VERTICAL, 0 );
	
	bSizer91->Add( bSizer121, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer141;
	bSizer141 = new wxBoxSizer( wxHORIZONTAL );
	
	wxArrayString m_program_cpu_deviceChoices;
	m_program_cpu_device = new wxChoice( m_panel_program, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_program_cpu_deviceChoices, 0 );
	m_program_cpu_device->SetSelection( 0 );
	m_program_cpu_device->SetMinSize( wxSize( 100,-1 ) );
	
	bSizer141->Add( m_program_cpu_device, 0, wxALL, 5 );
	
	wxArrayString m_program_cpu_paddingChoices;
	m_program_cpu_padding = new wxChoice( m_panel_program, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_program_cpu_paddingChoices, 0 );
	m_program_cpu_padding->SetSelection( 0 );
	m_program_cpu_padding->SetMinSize( wxSize( 60,-1 ) );
	
	bSizer141->Add( m_program_cpu_padding, 0, wxALL, 5 );
	
	m_program_cpu_value = new wxStaticText( m_panel_program, wxID_ANY, wxT("0x000000/0x000000"), wxDefaultPosition, wxDefaultSize, 0 );
	m_program_cpu_value->Wrap( -1 );
	m_program_cpu_value->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ) );
	
	bSizer141->Add( m_program_cpu_value, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer91->Add( bSizer141, 1, wxALIGN_RIGHT, 5 );
	
	m_staticline21 = new wxStaticLine( m_panel_program, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer91->Add( m_staticline21, 0, wxEXPAND | wxALL, 2 );
	
	wxBoxSizer* bSizer131;
	bSizer131 = new wxBoxSizer( wxHORIZONTAL );
	
	m_program_ppu_label = new wxStaticText( m_panel_program, wxID_ANY, wxT("Charcter flash"), wxDefaultPosition, wxDefaultSize, 0 );
	m_program_ppu_label->Wrap( -1 );
	m_program_ppu_label->SetMinSize( wxSize( 80,-1 ) );
	
	bSizer131->Add( m_program_ppu_label, 0, wxALL, 5 );
	
	m_program_ppu_gauge = new wxGauge( m_panel_program, wxID_ANY, 100, wxDefaultPosition, wxSize( -1,12 ), wxGA_HORIZONTAL );
	bSizer131->Add( m_program_ppu_gauge, 1, wxALL|wxALIGN_CENTER_VERTICAL, 0 );
	
	bSizer91->Add( bSizer131, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer1321;
	bSizer1321 = new wxBoxSizer( wxHORIZONTAL );
	
	wxArrayString m_program_ppu_deviceChoices;
	m_program_ppu_device = new wxChoice( m_panel_program, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_program_ppu_deviceChoices, 0 );
	m_program_ppu_device->SetSelection( 0 );
	m_program_ppu_device->SetMinSize( wxSize( 100,-1 ) );
	
	bSizer1321->Add( m_program_ppu_device, 0, wxALL, 5 );
	
	wxArrayString m_program_ppu_paddingChoices;
	m_program_ppu_padding = new wxChoice( m_panel_program, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_program_ppu_paddingChoices, 0 );
	m_program_ppu_padding->SetSelection( 0 );
	m_program_ppu_padding->SetMinSize( wxSize( 60,-1 ) );
	
	bSizer1321->Add( m_program_ppu_padding, 0, wxALL, 5 );
	
	m_program_ppu_value = new wxStaticText( m_panel_program, wxID_ANY, wxT("0x000000/0x000000"), wxDefaultPosition, wxDefaultSize, 0 );
	m_program_ppu_value->Wrap( -1 );
	m_program_ppu_value->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 76, 90, 90, false, wxEmptyString ) );
	
	bSizer1321->Add( m_program_ppu_value, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer91->Add( bSizer1321, 1, wxALIGN_RIGHT, 5 );
	
	m_staticline31 = new wxStaticLine( m_panel_program, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer91->Add( m_staticline31, 0, wxEXPAND | wxALL, 2 );
	
	wxBoxSizer* bSizer611;
	bSizer611 = new wxBoxSizer( wxHORIZONTAL );
	
	m_program_compare = new wxCheckBox( m_panel_program, wxID_ANY, wxT("&compare"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer611->Add( m_program_compare, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_program_button = new wxButton( m_panel_program, wxID_ANY, wxT("&program"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer611->Add( m_program_button, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	bSizer91->Add( bSizer611, 0, wxALIGN_RIGHT, 5 );
	
	m_panel_program->SetSizer( bSizer91 );
	m_panel_program->Layout();
	bSizer91->Fit( m_panel_program );
	m_notebook->AddPage( m_panel_program, wxT("program"), false );
	m_panel_wram = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	m_panel_wram->Hide();
	
	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer221;
	bSizer221 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticline111 = new wxStaticLine( m_panel_wram, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer221->Add( m_staticline111, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText221 = new wxStaticText( m_panel_wram, wxID_ANY, wxT("RAM read"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText221->Wrap( -1 );
	bSizer221->Add( m_staticText221, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticline71 = new wxStaticLine( m_panel_wram, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer221->Add( m_staticline71, 1, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	bSizer17->Add( bSizer221, 1, wxEXPAND, 2 );
	
	wxFlexGridSizer* fgSizer3;
	fgSizer3 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer3->AddGrowableCol( 1 );
	fgSizer3->SetFlexibleDirection( wxBOTH );
	fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText16 = new wxStaticText( m_panel_wram, wxID_ANY, wxT("&script"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText16->Wrap( -1 );
	fgSizer3->Add( m_staticText16, 0, wxALL, 5 );
	
	wxArrayString m_ram_read_scriptChoices;
	m_ram_read_script = new wxChoice( m_panel_wram, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_ram_read_scriptChoices, 0 );
	m_ram_read_script->SetSelection( 0 );
	fgSizer3->Add( m_ram_read_script, 0, wxALL|wxEXPAND, 5 );
	
	m_staticText17 = new wxStaticText( m_panel_wram, wxID_ANY, wxT("RAM image"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText17->Wrap( -1 );
	fgSizer3->Add( m_staticText17, 0, wxALL, 5 );
	
	m_ram_read_picker = new wxFilePickerCtrl( m_panel_wram, wxID_ANY, wxEmptyString, wxT("Select a file"), wxT("*.sav"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE|wxFLP_FILE_MUST_EXIST|wxFLP_USE_TEXTCTRL );
	fgSizer3->Add( m_ram_read_picker, 0, wxALL|wxEXPAND, 5 );
	
	bSizer17->Add( fgSizer3, 0, wxEXPAND, 2 );
	
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ram_read_gauge = new wxGauge( m_panel_wram, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	m_ram_read_gauge->SetMinSize( wxSize( -1,12 ) );
	
	bSizer18->Add( m_ram_read_gauge, 1, wxALL, 2 );
	
	m_ram_read_button = new wxButton( m_panel_wram, wxID_ANY, wxT("&read"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer18->Add( m_ram_read_button, 0, wxALL|wxALIGN_RIGHT, 5 );
	
	bSizer17->Add( bSizer18, 0, wxEXPAND, 2 );
	
	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticline12 = new wxStaticLine( m_panel_wram, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer22->Add( m_staticline12, 1, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticText22 = new wxStaticText( m_panel_wram, wxID_ANY, wxT("RAM write"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText22->Wrap( -1 );
	bSizer22->Add( m_staticText22, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5 );
	
	m_staticline7 = new wxStaticLine( m_panel_wram, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer22->Add( m_staticline7, 1, wxALL|wxALIGN_CENTER_VERTICAL, 2 );
	
	bSizer17->Add( bSizer22, 0, wxEXPAND, 2 );
	
	wxFlexGridSizer* fgSizer31;
	fgSizer31 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer31->AddGrowableCol( 1 );
	fgSizer31->SetFlexibleDirection( wxBOTH );
	fgSizer31->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText161 = new wxStaticText( m_panel_wram, wxID_ANY, wxT("s&cript"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText161->Wrap( -1 );
	fgSizer31->Add( m_staticText161, 0, wxALL, 5 );
	
	wxArrayString m_ram_write_scriptChoices;
	m_ram_write_script = new wxChoice( m_panel_wram, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_ram_write_scriptChoices, 0 );
	m_ram_write_script->SetSelection( 0 );
	fgSizer31->Add( m_ram_write_script, 0, wxALL|wxEXPAND, 5 );
	
	m_staticText171 = new wxStaticText( m_panel_wram, wxID_ANY, wxT("RAM image"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText171->Wrap( -1 );
	fgSizer31->Add( m_staticText171, 0, wxALL, 5 );
	
	m_ram_write_picker = new wxFilePickerCtrl( m_panel_wram, wxID_ANY, wxEmptyString, wxT("Select a file"), wxT("*.sav"), wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE|wxFLP_FILE_MUST_EXIST|wxFLP_SAVE|wxFLP_USE_TEXTCTRL );
	fgSizer31->Add( m_ram_write_picker, 0, wxALL|wxEXPAND, 5 );
	
	bSizer17->Add( fgSizer31, 0, wxEXPAND, 2 );
	
	wxBoxSizer* bSizer181;
	bSizer181 = new wxBoxSizer( wxHORIZONTAL );
	
	m_ram_write_gauge = new wxGauge( m_panel_wram, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	m_ram_write_gauge->SetMinSize( wxSize( -1,12 ) );
	
	bSizer181->Add( m_ram_write_gauge, 1, wxALL, 2 );
	
	m_ram_write_button = new wxButton( m_panel_wram, wxID_ANY, wxT("&write"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer181->Add( m_ram_write_button, 0, wxALL|wxALIGN_RIGHT, 5 );
	
	bSizer17->Add( bSizer181, 0, wxEXPAND, 2 );
	
	m_panel_wram->SetSizer( bSizer17 );
	m_panel_wram->Layout();
	bSizer17->Fit( m_panel_wram );
	m_notebook->AddPage( m_panel_wram, wxT("workram"), false );
	m_panel_version = new wxPanel( m_notebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer( wxVERTICAL );
	
	m_version_title = new wxStaticText( m_panel_version, wxID_ANY, wxT("famicom cartridge utility - anago"), wxDefaultPosition, wxDefaultSize, 0 );
	m_version_title->Wrap( -1 );
	m_version_title->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );
	
	bSizer30->Add( m_version_title, 0, wxALL, 2 );
	
	m_version_copyright = new wxStaticText( m_panel_version, wxID_ANY, wxT("(C) unagi development team 2010"), wxDefaultPosition, wxDefaultSize, 0 );
	m_version_copyright->Wrap( -1 );
	bSizer30->Add( m_version_copyright, 0, wxALL, 2 );
	
	wxBoxSizer* bSizer16;
	bSizer16 = new wxBoxSizer( wxHORIZONTAL );
	
	m_version_photo = new wxStaticBitmap( m_panel_version, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize( 178,122 ), 0 );
	m_version_photo->SetToolTip( wxT("okada") );
	
	bSizer16->Add( m_version_photo, 0, wxALL, 2 );
	
	m_version_developer = new wxStaticText( m_panel_version, wxID_ANY, wxT("programmer - naruko's latest photo\n\nicon designed by hirohiroki"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_version_developer->Wrap( 130 );
	bSizer16->Add( m_version_developer, 0, wxALL, 5 );
	
	bSizer30->Add( bSizer16, 1, wxEXPAND, 5 );
	
	m_version_detail = new wxTextCtrl( m_panel_version, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	m_version_detail->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVEBORDER ) );
	m_version_detail->SetMinSize( wxSize( -1,80 ) );
	
	bSizer30->Add( m_version_detail, 0, wxALL|wxEXPAND, 2 );
	
	m_panel_version->SetSizer( bSizer30 );
	m_panel_version->Layout();
	bSizer30->Fit( m_panel_version );
	m_notebook->AddPage( m_panel_version, wxT("version"), false );
	
	bSizer4->Add( m_notebook, 0, wxALL|wxEXPAND, 0 );
	
	m_panel_log = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );
	
	m_log = new wxTextCtrl( m_panel_log, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	m_log->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	
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
