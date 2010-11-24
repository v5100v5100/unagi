#include <wx/wx.h>
#include <wx/app.h>
#include <wx/thread.h>
#include <wx/dir.h>
#include <wx/sound.h>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <cassert>
#include <cstdarg>
#include "type.h"
#include "anago_gui.h"
#include "nescartxml.hh"
#include "widget.h"
#include "reader_master.h"
#include "reader_kazzo.h"
#include "reader_dummy.h"
extern "C"{
  #include "romimage.h"
  #include "flash_device.h"
  #include "script_dump.h"
  #include "script_program.h"
  void qr_version_print(const struct textcontrol *l);
}
#ifdef _UNICODE
  #define STRNCPY wcsncpy
#else
  #define STRNCPY strncpy
#endif
//---- C++ -> C -> C++ wrapping static functions ----
namespace c_wrapper{
	static void throw_error(const wxChar *t)
	{
		throw t;
	}

	static void value_set(void *gauge, void *label, int value)
	{
		wxGauge *g = static_cast<wxGauge *>(gauge);
		wxStaticText *l = static_cast<wxStaticText *>(label);
		wxString str;
		if(g->GetRange() == 1){
			str = wxT("skip             ");
		}else{
			str.Printf(wxT("0x%06x/0x%06x"), value, g->GetRange());
		}
		
		wxMutexGuiEnter();
		g->SetValue(value);
		if(label != NULL){
			l->SetLabel(str);
		}
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

	static void text_append_va(void *log, const wxChar *format, va_list list)
	{
		wxTextCtrl *l = static_cast<wxTextCtrl *>(log);
		wxString str;
		str.PrintfV(format, list);

		wxMutexGuiEnter();
		*l << str;
		wxMutexGuiLeave();
	}

	static void text_append(void *log, const wxChar *format, ...)
	{
		va_list list;
		va_start(list, format);
		text_append_va(log, format, list);
		va_end(list);
	}

	static void version_append_va(void *log, const wxChar *format, va_list list)
	{
		wxTextCtrl *l = static_cast<wxTextCtrl *>(log);
		wxString str;
		str.PrintfV(format, list);

		*l << str;
	}

	static void version_append(void *log, const wxChar *format, ...)
	{
		va_list list;
		va_start(list, format);
		version_append_va(log, format, list);
		va_end(list);
	}

	static void label_set(void *label, const wxChar *format, ...)
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

	static void choice_append(void *choice, const wxChar *str)
	{
		wxChoice *c = static_cast<wxChoice *>(choice);
		c->Append(wxString(str));
	}

	static void gauge_init(struct gauge *t)
	{
		t->label_set = label_set;
		t->range_set = range_set;
		t->value_set = value_set;
		t->value_add = value_add;
	}
}

//---- script execute thread ----
class anago_dumper : public wxThread
{
private:
	wxWindow *const m_frame;
	wxTextCtrl *const m_log;
	const wxSound m_sound_success, m_sound_fail;
	RomDb *const m_romdb;

	struct dump_config m_config;
protected:
	void *Entry(void)
	{
		try{
			bool r = false;
			switch(m_config.mode){
			case MODE_ROM_DUMP:
				r = script_dump_execute(&m_config);
				break;
			case MODE_RAM_READ: case MODE_RAM_WRITE:
				r = script_workram_execute(&m_config);
				break;
			default:
				assert(0);
				break;
			}
			if(r == true && m_sound_success.IsOk() == true){
				m_sound_success.Play();
			}
		}catch(const wxChar *t){
			if(m_sound_fail.IsOk() == true){
				m_sound_fail.Play();
			}
			*m_log << t;
		}
		if(m_romdb != NULL){
			m_romdb->Search(m_config.crc, m_log);
		}
		wxMutexGuiEnter();
		m_frame->Enable();
		wxMutexGuiLeave();
		return NULL;
	}
	void OnExit()
	{
		delete [] m_config.script;
		delete [] m_config.target;
	}
public:
	anago_dumper(wxWindow *f, const struct dump_config *d, wxTextCtrl *log, wxString sound_success, wxString sound_fail, RomDb *db = NULL) 
	  : wxThread(), m_frame(f), m_log(log), 
	  m_sound_success(sound_success), m_sound_fail(sound_fail),
	  m_romdb(db)
	{
		m_config = *d; //struct data copy
	}
};

class anago_programmer : public wxThread
{
private:
	wxWindow *const m_frame;
	wxTextCtrl *const m_log;
	struct program_config m_config;
	const wxSound m_sound_success, m_sound_fail;
protected:
	void *Entry(void)
	{
		try{
			if(script_program_execute(&m_config) == true){
				if(m_sound_success.IsOk() == true){
					m_sound_success.Play();
				}
			}
		}catch(const wxChar *t){
			if(m_sound_fail.IsOk() == true){
				m_sound_fail.Play();
			}
			*m_log << t;
		}
		wxMutexGuiEnter();
		m_frame->Enable();
		wxMutexGuiLeave();
		return NULL;
	}

	void OnExit()
	{
		delete [] m_config.script;
		delete [] m_config.target;
	}
public:
	anago_programmer(wxWindow *f, wxTextCtrl *log, const struct program_config *d, wxString sound_success, wxString sound_fail) 
	  : wxThread(), m_frame(f), m_log(log),
	  m_sound_success(sound_success), m_sound_fail(sound_fail)
	{
		m_config = *d;
	}
};

static void script_choice_init(wxControlWithItems *c, wxArrayString filespec, wxTextCtrl *log)
{
	wxDir dir(wxGetCwd());
	wxArrayString ar;

	c->Clear();
	if ( !dir.IsOpened() ){
		return;
	}
	for(size_t i = 0; i < filespec.GetCount(); i++){
		wxString filename;
		bool cont = dir.GetFirst(&filename, filespec[i], wxDIR_FILES);
		while ( cont ){
			ar.Add(filename);
			cont = dir.GetNext(&filename);
		}
	}
	if(ar.GetCount() == 0){
		*log << wxT("warning: ");
		for(size_t i = 0; i < filespec.GetCount(); i++){
			*log << filespec[i] << wxT(" ");
		}
		*log << wxT("script not found.\n");
	}else{
		ar.Sort(false);
		for(size_t i = 0; i < ar.GetCount(); i++){
			c->Append(ar[i]);
		}
		c->Select(0);
	}
}

static void increase_init(wxControlWithItems *c)
{
	c->Clear();
	c->Append(wxT("x1"));
	c->Append(wxT("x2"));
	c->Append(wxT("x4"));
	c->Select(0);
}

static int increase_get(wxControlWithItems *c)
{
	switch(c->GetSelection()){
	case 0: return 1;
	case 1: return 2;
	case 2: return 4;
	case 3: return INCREASE_AUTO;
	}
	return 1;
}

enum anago_status{
	STATUS_IDLE, STATUS_DUMPPING, STATUS_PROGRAMMING,
	STATUS_RAM_READ, STATUS_RAM_WRITE
};

class anago_panel_dump : public panel_dump
{
private:
	wxThread *m_anago_thread;
	const struct reader_driver *const m_reader;
	enum anago_status *const m_status;
	wxTextCtrl *const m_log;
	wxString m_sound_success, m_sound_fail;
	wxString m_database;
	RomDb *m_romdb;

	void execute(void)
	{
		struct dump_config config;
		config.mode = MODE_ROM_DUMP;
		config.cpu.gauge.bar = m_cpu_gauge;
		config.cpu.gauge.label = m_cpu_value;
		c_wrapper::gauge_init(&config.cpu.gauge);

		config.ppu.gauge.bar = m_ppu_gauge;
		config.ppu.gauge.label = m_ppu_value;
		c_wrapper::gauge_init(&config.ppu.gauge);
		
		config.log.object = m_log;
		config.log.append = c_wrapper::text_append;
		config.log.append_va = c_wrapper::text_append_va;
		config.except = c_wrapper::throw_error;
		config.cpu.increase = increase_get(m_cpu_increase);
		config.ppu.increase = increase_get(m_ppu_increase);
		config.progress = true;
		config.battery = m_check_battery->GetValue();
		{
			wxString str_script = m_script_choice->GetStringSelection();
			wxChar *t = new wxChar[str_script.Length() + 1];
			config.script = t;
			STRNCPY(t, str_script.fn_str(), str_script.Length() + 1);
		}

		{
			wxString str;
			config.mappernum = -1;
			if(m_check_forcemapper->GetValue() == true){
				str = m_text_forcemapper->GetValue();
				if(str.ToLong(&config.mappernum) == false){
					*m_log << wxT("bad mapper number\n");
					return;
				}
			}
		}

		{
			wxTextCtrl *text = m_romimage_picker->GetTextCtrl();
			wxString str_rom = text->GetValue();
			wxChar *t = new wxChar[str_rom.Length() + 1];
			if(text->IsEmpty() == true){
				*m_log << wxT("Enter filename to ROM image\n");
				return;
			}
			config.target = t;
			STRNCPY(t, str_rom.fn_str(), str_rom.Length() + 1);
		}

		config.control = &m_reader->control;
		config.cpu.access = &m_reader->cpu;
		config.ppu.access = &m_reader->ppu;

		this->Disable();

/*		if(m_anago_thread != NULL){ //???
			delete m_anago_thread;
		}*/
		m_anago_thread = new anago_dumper(this, &config, m_log, m_sound_success, m_sound_fail, m_romdb);
		if(m_anago_thread->Create() != wxTHREAD_NO_ERROR){
			*m_log << wxT("thread creating error");
		}else if(m_anago_thread->Run() != wxTHREAD_NO_ERROR){
			*m_log << wxT("thread running error");
		}else{
			*m_status = STATUS_DUMPPING;
		}
	}
protected:
	void button_click(wxCommandEvent& event)
	{
		switch(*m_status){
		case STATUS_IDLE:
			this->execute();
			break;
		case STATUS_DUMPPING:
			m_anago_thread->Kill();
			this->Enable();
			break;
		default: //do nothing
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

public:
	anago_panel_dump(wxNotebook *p, const struct reader_driver *r, enum anago_status *status, wxFileConfig *config, wxTextCtrl *log)
	  : panel_dump(p), m_reader(r), m_status(status), m_log(log)
	{
		config->Read(wxT("dump.database"), &m_database, wxT("NesCarts (2010-02-08).xml"));
		if(wxFileName::FileExists(m_database) == true){
			m_romdb = new RomDb(m_database);
			m_romdb->Generate();
		}else{
			*log << wxT("m_database not found\n");
			m_romdb = new RomDb();
		}

		config->Read(wxT("dump.sound.success"), &m_sound_success, wxT("tinkalink2.wav"));
		config->Read(wxT("dump.sound.fail"), &m_sound_fail, wxT("doggrowl.wav"));

		wxArrayString ar;
		ar.Add(wxT("*.ad"));
		ar.Add(wxT("*.ae"));
		ar.Add(wxT("*.af"));
		ar.Add(wxT("*.ag"));
		script_choice_init(m_script_choice, ar, log);
		::increase_init(m_cpu_increase);
		m_cpu_increase->Append(wxT("Auto"));
		m_cpu_increase->Select(3);
		::increase_init(m_ppu_increase);
		if(DEBUG==1){
			m_romimage_picker->GetTextCtrl()->SetLabel(wxT("t.nes"));
		}
	}
	virtual ~anago_panel_dump(void)
	{
		delete m_romdb;
	}
	bool Enable(bool t = true)
	{
		m_script_choice->Enable(t);
		m_romimage_picker->Enable(t);
		m_check_battery->Enable(t);
		m_check_forcemapper->Enable(t);
		m_cpu_increase->Enable(t);
		m_ppu_increase->Enable(t);
		if(t == true){
			*m_status = STATUS_IDLE;
			m_button->SetLabel(wxT("&dump"));
			if(m_check_forcemapper->GetValue() == true){
				m_text_forcemapper->Enable();
			}
		}else{
			m_button->SetLabel(wxT("cancel"));
			m_check_forcemapper->Disable();
		}
		m_button->SetFocus();
		return true;
	}
	bool Disable(void)
	{
		return this->Enable(false);
	}
};

class anago_panel_program : public panel_program
{
private:
	wxThread *m_anago_thread;
	const struct reader_driver *const m_reader;
	wxString m_sound_success, m_sound_fail;
	enum anago_status *const m_status;
	wxTextCtrl *const m_log;
	
	void padding_init(wxControlWithItems *c)
	{
		c->Clear();
		c->Append(wxT("full"));
		c->Append(wxT("top"));
		c->Append(wxT("bottom"));
		c->Append(wxT("empty"));
		c->Select(0);
	}
	bool rom_set(wxString device, int trans, struct memory *m, struct flash_device *f)
	{
		m->offset = 0;
		if(flash_device_get(device, f) == false){
			*m_log << wxT("unknown flash memory device ");
			*m_log << device << wxT("\n");
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

	void execute(void)
	{
		struct program_config f;
		
		f.cpu.gauge.bar = m_cpu_gauge;
		f.cpu.gauge.label = m_cpu_value;
		c_wrapper::gauge_init(&f.cpu.gauge);

		f.ppu.gauge.bar = m_ppu_gauge;
		f.ppu.gauge.label = m_ppu_value;
		c_wrapper::gauge_init(&f.ppu.gauge);
		
		f.log.object = m_log;
		f.log.append = c_wrapper::text_append;
		f.log.append_va = c_wrapper::text_append_va;
		f.except = c_wrapper::throw_error;
		
		{
			wxString str_script = m_script_choice->GetStringSelection();
			wxChar *t = new wxChar[str_script.Length() + 1];
			STRNCPY(t, str_script.fn_str(), str_script.Length() + 1);
			f.script = t;
		}

		{
			wxTextCtrl *text = m_romimage_picker->GetTextCtrl();
			wxString str_rom = text->GetValue();
			if(text->IsEmpty() == true){
				*m_log << wxT("Enter filename to ROM image\n");
				return;
			}
			wxChar *t = new wxChar[str_rom.Length() + 1];
			STRNCPY(t, str_rom.fn_str(), str_rom.Length() + 1);
			f.target = t;
		}
		f.compare = m_compare->GetValue();

		if(rom_set(
			m_cpu_device->GetStringSelection(), 
			m_cpu_padding->GetSelection(),
			&f.cpu.memory, &f.cpu.flash
		) == false){
			return;
		}
		if(rom_set(
			m_ppu_device->GetStringSelection(), 
			m_ppu_padding->GetSelection(),
			&f.ppu.memory, &f.ppu.flash
		) == false){
			return;
		}

		f.control = &m_reader->control;
		f.cpu.access = &m_reader->cpu;
		f.ppu.access = &m_reader->ppu;

		this->Disable();

		m_anago_thread = new anago_programmer(this, m_log, &f, m_sound_success, m_sound_fail);
		if(m_anago_thread->Create() != wxTHREAD_NO_ERROR){
			*m_log << wxT("thread creating error");
		}else if(m_anago_thread->Run() != wxTHREAD_NO_ERROR){
			*m_log << wxT("thread running error");
		}else{
			*m_status = STATUS_PROGRAMMING;
		}
	}

	void device_load(wxControlWithItems *choice, wxFileConfig *c, wxString key)
	{
		wxString device;
		int val;
		c->Read(key, &device);
		val = choice->FindString(device);
		if(val == wxNOT_FOUND){
			choice->Select(0);
		}else{
			choice->Select(val);
		}
	}
protected:
	void button_click(wxCommandEvent& event)
	{
		switch(*m_status){
		case STATUS_IDLE:
			this->execute();
			break;
		case STATUS_PROGRAMMING:
			m_anago_thread->Kill();
			this->Enable();
			break;
		default: //do nothing
			break;
		}
	}
public:
	anago_panel_program(wxNotebook *p, const struct reader_driver *r, enum anago_status *status, wxFileConfig *config, wxTextCtrl *log) 
	  : panel_program(p), m_reader(r), m_status(status), m_log(log)
	{
		config->Read(wxT("program.sound.success"), &m_sound_success, wxT("cuckoo.wav"));
		config->Read(wxT("program.sound.fail"), &m_sound_fail, wxT("doggrowl.wav"));
		{
			wxArrayString t;
			t.Add(wxT("*.af"));
			t.Add(wxT("*.ag"));
			script_choice_init(m_script_choice, t, m_log);
		}
		{
			struct flash_listup list;
			list.obj_cpu = m_cpu_device;
			list.obj_ppu = m_ppu_device;
			list.append = c_wrapper::choice_append;
			flash_device_listup(&list);
		}
		if(m_cpu_device->GetCount() == 0){
			*m_log << wxT("warning: flash device parameter not found\n");
		}else{
			device_load(m_cpu_device, config, wxT("program.cpu.device"));
			device_load(m_ppu_device, config, wxT("program.ppu.device"));
		}
		this->padding_init(m_cpu_padding);
		this->padding_init(m_ppu_padding);
		
		m_anago_thread = NULL;
	}
	bool Enable(bool t = true)
	{
		m_script_choice->Enable(t);
		m_romimage_picker->Enable(t);
		m_compare->Enable(t);
		m_cpu_padding->Enable(t);
		m_cpu_device->Enable(t);
		m_ppu_padding->Enable(t);
		m_ppu_device->Enable(t);
		if(t == true){
			*m_status = STATUS_IDLE;
			m_button->SetLabel(wxT("&program"));
		}else{
			m_button->SetLabel(wxT("cancel"));
		}
		m_button->SetFocus();
		return true;
	}
	bool Disable(void)
	{
		return this->Enable(false);
	}
	void ConfigWrite(wxFileConfig *c)
	{
		c->Write(wxT("program.cpu.device"), m_cpu_device->GetStringSelection());
		c->Write(wxT("program.ppu.device"), m_ppu_device->GetStringSelection());
	}
};

class anago_panel_workram : public panel_workram
{
private:
	wxThread *m_anago_thread;
	const struct reader_driver *const m_reader;
	enum anago_status *const m_status;
	wxTextCtrl *const m_log;
	wxString m_sound_success, m_sound_fail;

	void execute(struct dump_config *c, wxControlWithItems *script, wxFilePickerCtrl *picker, enum anago_status status)
	{
		c->cpu.gauge.label = NULL;
		c_wrapper::gauge_init(&c->cpu.gauge);
		
		c->log.object = m_log;
		c->log.append = c_wrapper::text_append;
		c->log.append_va = c_wrapper::text_append_va;
		c->except = c_wrapper::throw_error;
		assert(c->cpu.increase != INCREASE_AUTO);
		c->progress = true;
		{
			wxString str_script = script->GetStringSelection();
			wxChar *t = new wxChar[str_script.Length() + 1];
			c->script = t;
			STRNCPY(t, str_script.fn_str(), str_script.Length() + 1);
		}
		{
			wxTextCtrl *text = picker->GetTextCtrl();
			wxString str_rom = text->GetValue();
			wxChar *t = new wxChar[str_rom.Length() + 1];
			if(text->IsEmpty() == true){
				*m_log << wxT("Enter filename to ROM image\n");
				return;
			}
			c->target = t;
			STRNCPY(t, str_rom.fn_str(), str_rom.Length() + 1);
		}
		c->control = &m_reader->control;
		c->cpu.access = &m_reader->cpu;
		c->ppu.access = &m_reader->ppu;
		
		m_anago_thread = new anago_dumper(this, c, m_log, m_sound_success, m_sound_fail);
		if(m_anago_thread->Create() != wxTHREAD_NO_ERROR){
			*m_log << wxT("thread creating error");
		}else if(m_anago_thread->Run() != wxTHREAD_NO_ERROR){
			*m_log << wxT("thread running error");
		}else{
			*m_status = status; //先に status を設定すること
			this->Disable();
		}
	}

	void read(void)
	{
		struct dump_config config;
		config.mode = MODE_RAM_READ;
		config.cpu.gauge.bar = m_read_gauge;
		config.cpu.increase = ::increase_get(m_read_increase);
		this->execute(&config, m_read_script, m_read_picker, STATUS_RAM_READ);
	}
	void write(void)
	{
		struct dump_config config;
		config.mode = MODE_RAM_WRITE;
		config.cpu.gauge.bar = m_write_gauge;
		config.cpu.increase = ::increase_get(m_write_increase);
		this->execute(&config, m_write_script, m_write_picker, STATUS_RAM_WRITE);
	}
protected:
	void read_button_click(wxCommandEvent& event)
	{
		switch(*m_status){
		case STATUS_IDLE:
			read();
			break;
		case STATUS_RAM_READ:
			this->Enable();
			break;
		default: //do nothing
			break;
		}
	}

	void write_button_click(wxCommandEvent& event)
	{
		switch(*m_status){
		case STATUS_IDLE:
			write();
			break;
		case STATUS_RAM_WRITE:
			this->Enable();
			break;
		default: //do nothing
			break;
		}
	}
public:
	anago_panel_workram(wxWindow *p, const struct reader_driver *r, enum anago_status *status, wxFileConfig *config, wxTextCtrl *log)
	  : panel_workram(p), m_reader(r), m_status(status), m_log(log)
	{
		config->Read(wxT("workram.sound.success"), &m_sound_success, wxT("tinkalink2.wav"));
		config->Read(wxT("workram.sound.fail"), &m_sound_fail, wxT("doggrowl.wav"));

		wxArrayString ar;
		ar.Add(wxT("*.ae"));
		ar.Add(wxT("*.ag"));
		::script_choice_init(m_read_script, ar, log);
		::script_choice_init(m_write_script, ar, log);
		::increase_init(m_read_increase);
		::increase_init(m_write_increase);
		
		if(DEBUG == 1){
			m_read_picker->GetTextCtrl()->SetLabel(wxT("t.sav"));
		}
	}
	
	bool Enable(bool t = true)
	{
		m_read_script->Enable(t);
		m_read_picker->Enable(t);
		m_write_script->Enable(t);
		m_write_picker->Enable(t);
		if(t == true){
			switch(*m_status){
			case STATUS_RAM_READ:
				m_read_button->SetLabel(wxT("&read"));
				m_read_button->SetFocus();
				m_write_button->Enable();
				break;
			case STATUS_RAM_WRITE:
				m_write_button->SetLabel(wxT("&write"));
				m_write_button->SetFocus();
				m_read_button->Enable();
				break;
			default:
				break;
			}
			*m_status = STATUS_IDLE;
		}else{
			switch(*m_status){
			case STATUS_RAM_READ:
				m_read_button->SetLabel(wxT("&cancel"));
				m_write_button->Disable();
				break;
			case STATUS_RAM_WRITE:
				m_write_button->SetLabel(wxT("&cancel"));
				m_read_button->Disable();
				break;
			default:
				break;
			}
		}
		return true;
	}

	bool Disable(void)
	{
		return this->Enable(false);
	}
};

class anago_panel_version : public panel_version
{
public:
	anago_panel_version(wxWindow *p) : panel_version(p)
	{
//version infomation
		{
			struct textcontrol detail;
			*m_version_detail << wxT("anago build at ") << wxT(__DATE__) << wxT("\n\n");
			detail.object = m_version_detail;
			detail.append = c_wrapper::version_append;
			detail.append_va = c_wrapper::version_append_va;
			qr_version_print(&detail);
			*m_version_detail << wxVERSION_STRING << wxT(" (c) Julian Smar");
		}
		
#if 0 //def WIN32
		#include "okada.xpm"
		wxBitmap bitmap_okada(okada);
		wxString tooltip(wxT(
			"緑区 na6ko 町さん (28歳, 童貞)\n\n"

			"28年間バカにされっぱなし、ミジメ過ぎた俺の人生が anago,\n"
			"kazzo を持つようになった途端、突然ツキがめぐってきた。\n"
//			"競馬をやれば連戦連勝、夢にまでみた万馬券を当て、気がつくと\n"
//			"しんじられない事にギャンブルで稼いだお金が460万円!!\n"
			"元手はたった4000円。しかもたった2ヶ月で人生大逆転!!\n"
			"女は3P4Pヤリ放題!!"
//			"勤めていた新聞屋も辞めギャンブルで\n"
//			"身を立てていこうと思っています。実は来月の11日にラスベガスに\n"
//			"行き勝負をかけます。結果はまた報告します。宜しく。"
		));
#else
		#include "taiyo.xpm"
		wxBitmap bitmap_okada(taiyo);
		wxString tooltip(wxT("たいよ～ほえ～るず♪"));
#endif
//		#include "araki.xpm"
//		wxBitmap bitmap_okada(araki);
		m_version_photo->SetBitmap(bitmap_okada);
		m_version_photo->SetToolTip(tooltip);
	}
};

//---- main frame class ----
class anago_frame : public frame_main
{
private:
	enum anago_status m_status; //ここだけ実体, 各パネルはこのポインタ
	const wxString m_config_file;
	anago_panel_program *m_panel_program;
protected:
	void menu_log_clean(wxCommandEvent& event)
	{
		m_log->Clear();
	}
	
public:
	/** Constructor */
	anago_frame(wxWindow* parent, const struct reader_driver *r)
	  : frame_main(parent), m_status(STATUS_IDLE),
#ifdef WIN32
	  m_config_file(wxGetCwd() + wxT("/anago.cfg"))
#else
	  m_config_file(wxT(".anago"))
#endif
	{
		wxFileConfig config(wxEmptyString, wxEmptyString, m_config_file);
//form config load
		wxPoint position;
		
		config.Read(wxT("position.x"), &position.x, 32);
		config.Read(wxT("position.y"), &position.y, 32);
		this->SetPosition(position);
		
		wxSize size;
		config.Read(wxT("size.x"), &size.x, 340);
		config.Read(wxT("size.y"), &size.y, 460);
		this->SetSize(size);

		m_notebook->AddPage(new anago_panel_dump(m_notebook, r, &m_status, &config, m_log), wxT("dump"), false);

		m_panel_program = new anago_panel_program(m_notebook, r, &m_status, &config, m_log);
		m_notebook->AddPage(m_panel_program, wxT("program"), false);

		m_notebook->AddPage(new anago_panel_workram(m_notebook, r, &m_status, &config, m_log), wxT("workram"), false);
		m_notebook->AddPage(new anago_panel_version(m_notebook), wxT("version"), false);

#ifdef WIN32
		SetIcon(wxIcon(wxT("unagi_blue")));
#else
		#include "unagi_blue.xpm"
		SetIcon(wxIcon(unagi_blue));
#endif
	}

	virtual ~anago_frame(void)
	{
		wxFileConfig config(wxEmptyString, wxEmptyString, m_config_file);
		wxPoint position = this->GetPosition();
		
		config.Write(wxT("position.x"), position.x);
		config.Write(wxT("position.y"), position.y);

		wxSize size = this->GetSize();
		config.Write(wxT("size.x"), size.x);
		config.Write(wxT("size.y"), size.y);
		
		m_panel_program->ConfigWrite(&config);
	}
};

#ifndef WIN32
extern "C"{
  int anago_cui(int c, wxChar **v);
}
int main(int c, wxChar **v)
{
	if(c < 3){
		return wxEntry(c, v);
	}
	return anago_cui(c, v);
}
#endif

class MyApp : public wxApp
{
private:
	anago_frame *m_frame;
public: 
	bool OnInit()
	{
		if(this->argc >= 2){
			m_frame = new anago_frame(NULL, &DRIVER_DUMMY);
		}else{
			m_frame = new anago_frame(NULL, &DRIVER_KAZZO);
		}
		m_frame->Show();
		
		return true;
	}
};
#ifdef WIN32
IMPLEMENT_APP(MyApp)
#else
IMPLEMENT_APP_NO_MAIN(MyApp)
#endif
