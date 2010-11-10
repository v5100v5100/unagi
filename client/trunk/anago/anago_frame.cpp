#include <wx/wx.h>
#include <wx/app.h>
#include <wx/thread.h>
#include <wx/dir.h>
#include <wx/sound.h>
#include <wx/fileconf.h>
#include <cstdarg>
#include "type.h"
#include "anago_gui.h"
#include "nescartxml.hh"
#include "widget.h"
#include "reader_master.h"
#include "reader_kazzo.h"
extern "C"{
  #include "romimage.h"
  #include "flash_device.h"
  #include "script_dump.h"
  #include "script_program.h"
  void qr_version_print(const struct textcontrol *l);
}
extern const struct reader_driver DRIVER_DUMMY;
#ifdef _UNICODE
  #define STRNCPY wcsncpy
#else
  #define STRNCPY strncpy
#endif
//---- C++ -> C -> C++ wrapping functions ----
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

//---- script execute thread ----
class anago_frame;
class anago_panel_dump;
class anago_panel_program;

class anago_dumper : public wxThread
{
private:
	anago_panel_dump *m_frame;
	struct dump_config m_config;
	const wxSound m_sound_success, m_sound_fail;
protected:
	void *Entry(void);
	void OnExit()
	{
		delete [] m_config.script;
		delete [] m_config.target;
	}
public:
	anago_dumper(anago_panel_dump *f, const struct dump_config *d, wxString sound_success, wxString sound_fail) 
	  : wxThread(), m_sound_success(sound_success), m_sound_fail(sound_fail)
	{
		m_frame = f;
		m_config = *d; //struct data copy
	}
};

class anago_programmer : public wxThread
{
private:
	anago_panel_program *m_frame;
	struct program_config m_config;
	const wxSound m_sound_success, m_sound_fail;
protected:
	void *Entry(void);
	void OnExit()
	{
		delete [] m_config.script;
		delete [] m_config.target;
	}
public:
	anago_programmer(anago_panel_program *f, const struct program_config *d, wxString sound_success, wxString sound_fail) 
	  : wxThread(), m_sound_success(sound_success), m_sound_fail(sound_fail)
	{
		m_frame = f;
		m_config = *d;
	}
};

static void script_choice_init(wxControlWithItems *c, wxString filespec, wxTextCtrl *log)
{
	wxDir dir(wxGetCwd());
	wxString filename;
	wxArrayString ar;

	c->Clear();
	if ( !dir.IsOpened() ){
		return;
	}
	bool cont = dir.GetFirst(&filename, filespec, wxDIR_FILES);
	while ( cont ){
		ar.Add(filename);
		cont = dir.GetNext(&filename);
	}
	if(ar.GetCount() == 0){
		*log << wxT("warning: ") << filespec << wxT(" script not found.\n");
	}else{
		ar.Sort(false);
		for(size_t i = 0; i < ar.GetCount(); i++){
			c->Append(ar[i]);
		}
		c->Select(0);
	}
}

void gauge_init(struct gauge *t)
{
	t->label_set = label_set;
	t->range_set = range_set;
	t->value_set = value_set;
	t->value_add = value_add;
}

enum anago_status{
	STATUS_IDLE, STATUS_DUMPPING, STATUS_PROGRAMMING,
	STATUS_RAM_READ, STATUS_RAM_WRITE
};

class anago_panel_dump : public panel_dump
{
private:
	wxThread *m_anago_thread;
	wxTextCtrl *m_log;
	const struct reader_driver *m_reader;
	wxString m_dump_sound_success, m_dump_sound_fail;
	RomDb *m_romdb;
	enum anago_status m_status;

	void dump_increase_init(wxControlWithItems *c)
	{
		c->Clear();
		c->Append(wxT("x1"));
		c->Append(wxT("x2"));
		c->Append(wxT("x4"));
		c->Select(0);
	}
	int dump_increase_get(wxControlWithItems *c)
	{
		switch(c->GetSelection()){
		case 0: return 1;
		case 1: return 2;
		case 2: return 4;
		case 3: return INCREASE_AUTO;
		}
		return 1;
	}
	void dump_execute(void)
	{
		struct dump_config config;
		config.mode = MODE_ROM_DUMP;
		config.cpu.gauge.bar = m_dump_cpu_gauge;
		config.cpu.gauge.label = m_dump_cpu_value;
		gauge_init(&config.cpu.gauge);

		config.ppu.gauge.bar = m_dump_ppu_gauge;
		config.ppu.gauge.label = m_dump_ppu_value;
		gauge_init(&config.ppu.gauge);
		
		config.log.object = m_log;
		config.log.append = text_append;
		config.log.append_va = text_append_va;
		config.except = throw_error;
		config.cpu.increase = dump_increase_get(m_dump_cpu_increase);
		config.ppu.increase = dump_increase_get(m_dump_ppu_increase);
		config.progress = true;
		config.battery = m_dump_check_battery->GetValue();
		{
			wxString str_script = m_dump_script_choice->GetStringSelection();
			wxChar *t = new wxChar[str_script.Length() + 1];
			config.script = t;
			STRNCPY(t, str_script.fn_str(), str_script.Length() + 1);
		}

		{
			wxString str;
			config.mappernum = -1;
			if(m_dump_check_forcemapper->GetValue() == true){
				str = m_dump_text_forcemapper->GetValue();
				if(str.ToLong(&config.mappernum) == false){
					*m_log << wxT("bad mapper number\n");
					return;
				}
			}
		}

		{
			wxTextCtrl *text = m_dump_romimage_picker->GetTextCtrl();
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

		m_dump_script_choice->Disable();
		m_dump_romimage_picker->Disable();
		m_dump_check_battery->Disable();
		m_dump_check_forcemapper->Disable();
		m_dump_button->SetLabel(wxT("cancel"));
		m_dump_text_forcemapper->Disable();
		m_dump_cpu_increase->Disable();
		m_dump_ppu_increase->Disable();

/*		if(m_anago_thread != NULL){ //???
			delete m_anago_thread;
		}*/
		m_anago_thread = new anago_dumper(this, &config, m_dump_sound_success, m_dump_sound_fail);
		if(m_anago_thread->Create() != wxTHREAD_NO_ERROR){
			*m_log << wxT("thread creating error");
		}else if(m_anago_thread->Run() != wxTHREAD_NO_ERROR){
			*m_log << wxT("thread running error");
		}else{
//			m_status = STATUS_DUMPPING;
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

	void mapper_change_check(wxCommandEvent& event)
	{
		if(m_dump_check_forcemapper->GetValue() == true){
			m_dump_text_forcemapper->Enable();
		}else{
			m_dump_text_forcemapper->Disable();
		}
	}
public:
	anago_panel_dump(wxWindow *p, const struct reader_driver *r, wxFileConfig *config, wxTextCtrl *log) : panel_dump(p), m_status(STATUS_IDLE)
	{
		m_reader = r;
		m_log = log;
		m_romdb = new RomDb(wxT("NesCarts (2010-02-08).xml"));
		m_romdb->Generate();

		config->Read(wxT("dump.sound.success"), &m_dump_sound_success, wxT("tinkalink2.wav"));
		config->Read(wxT("dump.sound.fail"), &m_dump_sound_fail, wxT("doggrowl.wav"));

		script_choice_init(m_dump_script_choice, wxT("*.ad"), log);
		this->dump_increase_init(m_dump_cpu_increase);
		m_dump_cpu_increase->Append(wxT("Auto"));
		m_dump_cpu_increase->Select(3);
		this->dump_increase_init(m_dump_ppu_increase);
		if(DEBUG==1){
			m_dump_romimage_picker->GetTextCtrl()->SetLabel(wxT("t.nes"));
		}
	}
	void DumpThreadFinish(unsigned long crc)
	{
		m_romdb->Search(crc, m_log);
		this->DumpThreadFinish();
	}
	void DumpThreadFinish(void)
	{
		m_dump_script_choice->Enable();
		m_dump_script_choice->SetFocus();
		m_dump_romimage_picker->Enable();
		m_dump_check_battery->Enable();
		m_dump_check_forcemapper->Enable();
		m_dump_cpu_increase->Enable();
		m_dump_ppu_increase->Enable();
		m_dump_button->SetLabel(wxT("&dump"));
		if(m_dump_check_forcemapper->GetValue() == true){
			m_dump_text_forcemapper->Enable();
		}
		m_status = STATUS_IDLE;
	}
	void LogAppend(const wxChar *t)
	{
		*m_log << t;
	}
};

void *anago_dumper::Entry(void)
{
	try{
		if(script_dump_execute(&m_config) == true){
			if(m_sound_success.IsOk() == true){
				m_sound_success.Play();
			}
		}
	}catch(const wxChar *t){
		if(m_sound_fail.IsOk() == true){
			m_sound_fail.Play();
		}
		m_frame->LogAppend(t);
	}
	m_frame->DumpThreadFinish(m_config.crc);
	return NULL;
}

class anago_panel_program : public panel_program
{
private:
	wxThread *m_anago_thread;
	const struct reader_driver *m_reader;
	wxString m_program_sound_success, m_program_sound_fail;
	enum anago_status m_status;
	wxTextCtrl *m_log;
	void program_padding_init(wxControlWithItems *c)
	{
		c->Clear();
		c->Append(wxT("full"));
		c->Append(wxT("top"));
		c->Append(wxT("bottom"));
		c->Append(wxT("empty"));
		c->Select(0);
	}
	bool program_rom_set(wxString device, int trans, struct memory *m, struct flash_device *f)
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
		f.except = throw_error;
		
		{
			wxString str_script = m_program_script_choice->GetStringSelection();
			wxChar *t = new wxChar[str_script.Length() + 1];
			STRNCPY(t, str_script.fn_str(), str_script.Length() + 1);
			f.script = t;
		}

		{
			wxTextCtrl *text = m_program_romimage_picker->GetTextCtrl();
			wxString str_rom = text->GetValue();
			if(text->IsEmpty() == true){
				*m_log << wxT("Enter filename to ROM image\n");
				return;
			}
			wxChar *t = new wxChar[str_rom.Length() + 1];
			STRNCPY(t, str_rom.fn_str(), str_rom.Length() + 1);
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

		f.control = &m_reader->control;
		f.cpu.access = &m_reader->cpu;
		f.ppu.access = &m_reader->ppu;

		m_program_script_choice->Disable();
		m_program_romimage_picker->Disable();
		m_program_compare->Disable();
		m_program_button->SetLabel(wxT("cancel"));
		m_program_cpu_padding->Disable();
		m_program_cpu_device->Disable();
		m_program_ppu_padding->Disable();
		m_program_ppu_device->Disable();
		m_program_compare->Disable();

		m_anago_thread = new anago_programmer(this, &f, m_program_sound_success, m_program_sound_fail);
		if(m_anago_thread->Create() != wxTHREAD_NO_ERROR){
			*m_log << wxT("thread creating error");
		}else if(m_anago_thread->Run() != wxTHREAD_NO_ERROR){
			*m_log << wxT("thread running error");
		}else{
			m_status = STATUS_PROGRAMMING;
		}
	}

	void program_device_load(wxControlWithItems *choice, wxFileConfig *c, wxString key)
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
public:
	anago_panel_program(wxWindow *p, const struct reader_driver *r, wxFileConfig *config, wxTextCtrl *log) : panel_program(p), m_status(STATUS_IDLE)
	{
		m_reader = r;
		m_log = log;
		config->Read(wxT("program.sound.success"), &m_program_sound_success, wxT("cuckoo.wav"));
		config->Read(wxT("program.sound.fail"), &m_program_sound_fail, wxT("doggrowl.wav"));
		script_choice_init(m_program_script_choice, wxT("*.af"), m_log);
		{
			struct flash_listup list;
			list.obj_cpu = m_program_cpu_device;
			list.obj_ppu = m_program_ppu_device;
			list.append = choice_append;
			flash_device_listup(&list);
		}
		if(m_program_cpu_device->GetCount() == 0){
			*m_log << wxT("warning: flash device parameter not found\n");
		}else{
			program_device_load(m_program_cpu_device, config, wxT("program.cpu.device"));
			program_device_load(m_program_ppu_device, config, wxT("program.ppu.device"));
		}
		this->program_padding_init(m_program_cpu_padding);
		this->program_padding_init(m_program_ppu_padding);
		
		m_anago_thread = NULL;
		m_status = STATUS_IDLE;
	}
	void ProgramThreadFinish(void)
	{
		m_program_script_choice->Enable();
		m_program_romimage_picker->Enable();
		m_program_compare->Enable();
		m_program_button->SetLabel(wxT("&program"));
		m_program_cpu_padding->Enable();
		m_program_cpu_device->Enable();
		m_program_ppu_padding->Enable();
		m_program_ppu_device->Enable();
		m_status = STATUS_IDLE;
	}
	void LogAppend(const wxChar *t)
	{
		*m_log << t;
	}
	void ConfigWrite(wxFileConfig *c)
	{
		c->Write(wxT("program.cpu.device"), m_program_cpu_device->GetStringSelection());
		c->Write(wxT("program.ppu.device"), m_program_ppu_device->GetStringSelection());
	}
};

void *anago_programmer::Entry(void)
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
		m_frame->LogAppend(t);
	}
	m_frame->ProgramThreadFinish();
	return NULL;
}

class anago_panel_workram : public panel_workram
{
private:
//	wxTextCtrl *m_log;
protected:
	void read_button_click(wxCommandEvent& event)
	{
	}
public:
	anago_panel_workram(wxWindow *p, wxTextCtrl *log) : panel_workram(p)
	{
//		m_log = log;
		script_choice_init(m_ram_read_script, wxT("*.ad"), log);
		script_choice_init(m_ram_write_script, wxT("*.ad"), log);
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
			detail.append = version_append;
			detail.append_va = version_append_va;
			qr_version_print(&detail);
			*m_version_detail << wxVERSION_STRING << wxT(" (c) Julian Smar");
		}
		
#ifdef WIN32
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
	  : frame_main(parent), 
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

		m_notebook->AddPage(new anago_panel_dump(m_notebook, r, &config, m_log), wxT("dump"), false);
		m_panel_program = new anago_panel_program(m_notebook, r, &config, m_log);
		m_notebook->AddPage(m_panel_program, wxT("program"), false);
		m_notebook->AddPage(new anago_panel_workram(m_notebook, m_log), wxT("workram"), false);
		m_notebook->AddPage(new anago_panel_version(m_notebook), wxT("version"), false);
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
