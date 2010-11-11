#include <wx/wx.h>
#include <wx/xml/xml.h>
#include "nescartxml.hh"

WX_DECLARE_HASH_MAP(unsigned long, wxXmlNode *, wxIntegerHash, wxIntegerEqual, CartridgeHash);

RomDb::RomDb(wxString file)
{
	m_hash = new CartridgeHash(2200);
	m_document = new wxXmlDocument(file);
}

RomDb::RomDb(void)
{
	m_hash = new CartridgeHash(20);
	m_document = new wxXmlDocument();
}

RomDb::~RomDb(void)
{
	delete m_hash;
	delete m_document;
}

bool RomDb::Generate(void)
{
	wxXmlNode *root = m_document->GetRoot();
	if(root == NULL){
		return false;
	}
	wxXmlNode *game = root->GetChildren();
	if(game == NULL){
		return false;
	}
	while(game != NULL){
		wxXmlNode *cartridge = game->GetChildren();
		while(cartridge != NULL){
			wxString crcstr;
			if(cartridge->GetPropVal(wxT("crc"), &crcstr) == true){
				unsigned long crc;
				if(crcstr.ToULong(&crc, 0x10) == true){
					(*m_hash)[crc] = cartridge;
				}
			}
			cartridge = cartridge->GetNext();
		}
		game = game->GetNext();
	}
/*	wxXmlNode *board = cartdige->GetChildren();
	wxXmlNode *parts = board->GetChildren();
	while(parts != NULL){
		wxString name = parts->GetName();
		if(name == wxT("prg") || name == wxT("chr")){
			*log << parts->GetPropVal(wxT("name"), wxT(""));
			*log << parts->GetPropVal(wxT("crc"), wxT("xx")) << wxT("\n");
		}
		parts = parts->GetNext();
	}*/
	return true;
}

void RomDb::Search(unsigned long crc, wxTextCtrl *log) const
{
	wxXmlNode *cartridge = (*m_hash)[crc];
	if(cartridge == NULL){
		return;
	}
	wxXmlNode *game = cartridge->GetParent();
	const wxString error = wxT("*error*");
	wxString field, name;
	if(game->GetPropVal(wxT("altname"), &name) == false){
		name = game->GetPropVal(wxT("name"), error);
	}
	field = wxT("name:") + name;
	*log << field << wxT("\n");
	
	field = wxT("region:") + game->GetPropVal(wxT("region"), error);
	field += wxT(" catalog:") + game->GetPropVal(wxT("catalog"), error);
	field += wxT(" revision:") + cartridge->GetPropVal(wxT("revision"), wxT("(none)"));
	*log << field << wxT("\n");
}

