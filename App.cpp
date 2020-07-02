#include "App.h"

wxIMPLEMENT_APP(App); //generate main function

App::App()
{

}

App::~App()
{

}

bool App::OnInit()
{
	m_frame1 = new Main(); //new instance of main
	m_frame1->Show(); //show it because main is a window
	//m_frame1->SetBackgroundColour(*wxWHITE);
	//m_frame1->SetForegroundColour(*wxWHITE);

	return true;
}