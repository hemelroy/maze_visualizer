#pragma once
#include "wx/wx.h"
#include <iostream>
#include <stack>
using namespace std;

// Event IDs
const int EVT_INCREASE_GRID = 20001;
const int EVT_DECREASE_GRID = 20000;
const int EVT_GENERATE_MAZE = 20111;

//inheritance makes this class a form 
class Main : public wxFrame
{
public:
	Main();
	~Main();

private:
	// == CONSTANTS ==
	const int MAZE_STARTING_HEIGHT = 100; //location of top border of field 
	const int MAZE_STARTING_WIDTH = 20;
 

	// == GUI widgets ==
	wxStaticText* txt_title = nullptr;
	wxStaticText* txt_subtitle = nullptr;
	wxStaticText* txt_sizeChange = nullptr;

	wxButton* btn_inc = nullptr;
	wxButton* btn_dec = nullptr;
	wxButton* btn_start = nullptr;

	//Maze display
	wxButton** fieldBtns;
	wxButton** topBorderBtns;
	wxButton** bottomBorderBtns;
	wxButton** leftBorderBtns;
	wxButton** rightBorderBtns;

	// Maze Algorithm Variables
	enum
	{
		CELL_PATH_N = 0x01,
		CELL_PATH_E = 0x02,
		CELL_PATH_S = 0x04,
		CELL_PATH_W = 0x08,
		CELL_VISITED = 0x10,
	};
	int visitedNodes = 0;
	int mazeHeight = 10;
	int mazeWidth = 15;
	int* maze; // Actual maze used in algorithm
	stack<pair<int, int>> m_stack;	// Maze stack storing (x, y) coordinate pairs
	int m_nPathWidth = 1;

	int fieldWidth = mazeWidth*2;
	int fieldHeight = mazeHeight*2-1;
	
	// Completion tracking
	int topCounter = 0;
	int previousTop = 0;
	bool isComplete = false;
	bool isCleared = true; // Tracks if maze has been cleared on the field

	// == Methods == 
	void OnButtonClicked(wxCommandEvent& evt);
	void updateGridSize(int oldFieldHeight, int oldFieldWidth, bool isDecreased);
	void generateMaze(void);
	void drawToField(int x, int y, int col);
	void initializeMaze();
	void drawField();

	wxDECLARE_EVENT_TABLE();

};

