#pragma once
#include "wx/wx.h"
#include <iostream>
#include <stack>
using namespace std;

// Event IDs
const int EVT_INCREASE_GRID = 20001;
const int EVT_DECREASE_GRID = 20000;
const int EVT_GENERATE_MAZE = 20111;
const int EVT_SOLVE_MAZE = 22222;

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
	wxButton* btn_solve = nullptr;

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
	bool isGenerated = false;
	bool isCleared = true; // Tracks if maze has been cleared on the field

	// Solver components
	struct mazeNode // each cell in the maze is a "node" in the solver
	{
		bool isObstacle = false;
		bool beenVisited = false;
		float globalGoal;
		float localGoal;
		int x; //node position in 2d space given by (x,y)
		int y;
		vector<mazeNode*> vecNeighbours;
		mazeNode* parent; //node connecting this node that offers 
	};

	mazeNode* nodeList = nullptr;
	mazeNode* startingNode = nullptr;
	mazeNode* endingNode = nullptr;

	// == Methods == 
	void OnButtonClicked(wxCommandEvent& evt); // main event handler for buttons
	void updateGridSize(int oldFieldHeight, int oldFieldWidth, bool isDecreased); // rescales maze dimensions
	void generateMaze(void); // generate new randomized maze
	void drawToField(int x, int y, int col); // draw maze onto screen
	void initializeMaze(); // initialize all maze properties
	void drawField(); // draw on-screen field 

	void initializeSolver(void); // initialize node properties and construct graph
	void solveMaze(void);

	wxDECLARE_EVENT_TABLE();

};

