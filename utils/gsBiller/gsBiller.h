#ifndef GSBILLER_H
#define GSBILLER_H

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------
    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();

};

class gsBiller: public wxFrame
{
	public:

		gsBiller(wxWindow* parent,wxWindowID id, const wxString title);
		virtual ~gsBiller();

#if 0
		//(*Declarations(gsBiller)
		wxMessageDialog* AboutInfo;
		wxMenuItem* MenuItem2;
		wxMenu* Menu3;
		wxMenuItem* MenuItem1;
		wxMenuItem* MenuItem4;
		wxFileDialog* FileDialog1;
		wxMenu* Menu1;
		wxMenuItem* MenuItem3;
		wxStatusBar* StatusBar1;
		wxProgressDialog* ProgressDialog1;
		wxMenuBar* MenuBar1;
		wxMenu* Menu2;
		wxTimer Timer1;
		//*)
#endif

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

	private:

		//(*Handlers(gsBiller)
		//*)

    DECLARE_EVENT_TABLE();
};

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
enum
{
    // menu items
    Minimal_Quit = wxID_EXIT,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Minimal_About = wxID_ABOUT,

    ID_MENUITEM1 = 10,
    ID_MENUITEM2,
    ID_MENUITEM3,
    ID_MENUITEM4,
    ID_STATUSBAR1,
    ID_PROGRESSDIALOG1,
    ID_TIMER1
};

#endif
