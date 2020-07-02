#include "Main.h"
#include <stack>
#include <thread>

wxBEGIN_EVENT_TABLE(Main, wxFrame)
wxEND_EVENT_TABLE()

//override constructor of base class
Main::Main() : wxFrame(nullptr, wxID_ANY, "Hemel Roy - Maze Visualizer", wxPoint(30, 30), wxSize(1300, 800))
{	
	//Generate GUI widgets
	txt_title = new wxStaticText(this, wxID_ANY, "Maze Algorithm Visualization Tool", wxPoint(100, 10), wxSize(300, 35));
	wxFont titleFont(16, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
	txt_title->SetFont(titleFont);
	txt_subtitle = new wxStaticText(this, wxID_ANY, "created by Hemel Roy", wxPoint(250, 40), wxSize(300, 30));
	txt_sizeChange = new wxStaticText(this, wxID_ANY, "Increase/decrease field size", wxPoint(1025, 50), wxSize(200, 25));

	btn_dec = new wxButton(this, EVT_DECREASE_GRID, "-", wxPoint(1050, 75), wxSize(50, 25));
	btn_dec->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Main::OnButtonClicked, this);
	btn_inc = new wxButton(this, EVT_INCREASE_GRID, "+", wxPoint(1100, 75), wxSize(50, 25));
	btn_inc->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Main::OnButtonClicked, this);
	btn_start = new wxButton(this, EVT_GENERATE_MAZE, "Generate Maze", wxPoint(1050, 150), wxSize(100, 25));
	btn_start->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Main::OnButtonClicked, this);
	
	//Generate inital maze field skeleton
	fieldBtns = new wxButton * [fieldWidth * fieldHeight];
	initializeMaze();
	drawField();
	
}

Main::~Main()
{
	delete[] fieldBtns;
	delete[] topBorderBtns;
	delete[] bottomBorderBtns;
	delete[] leftBorderBtns;
	delete[] rightBorderBtns;
	delete[] maze;
}

//Event handler
void Main::OnButtonClicked(wxCommandEvent& evt)
{
	
	//Get coordinate of button in field array
	//int x = (evt.GetId() - 10000) % nFieldWidth;
	//int y = (evt.GetId() - 10000) / nFieldWidth;
	int evtID = int(evt.GetId());

	switch (evtID)
	{
	case EVT_INCREASE_GRID:
		if (mazeWidth < 21)
		{
			mazeWidth++;
			mazeHeight++;
			int oldFieldWidth = fieldWidth;
			int oldFieldHeight = fieldHeight;
			fieldWidth = mazeWidth * 2;
			fieldHeight = mazeHeight * 2 - 1;
			updateGridSize(oldFieldHeight, oldFieldWidth, false);
		}
		break;
	case EVT_DECREASE_GRID:
		if (fieldHeight > 10)
		{
			mazeWidth--;
			mazeHeight--;
			int oldFieldWidth = fieldWidth;
			int oldFieldHeight = fieldHeight;
			fieldWidth = mazeWidth * 2;
			fieldHeight = mazeHeight * 2 - 1;
			updateGridSize(oldFieldHeight, oldFieldWidth, true);
		}
		break;
	case EVT_GENERATE_MAZE:
		if (!isCleared)
			updateGridSize(fieldHeight, fieldWidth, false);

		while (!isComplete)
		{
			generateMaze();
			this_thread::sleep_for(1ms);
			wxYield(); //refresh screen
		}
		isCleared = false;

		break;
	default:
		break;
	}

	evt.Skip();
}

void Main::updateGridSize(int oldFieldHeight, int oldFieldWidth, bool isDecreased)
{
	// Destory maze
	for (int x = 0; x < oldFieldWidth; x++)
	{
		for (int y = 0; y < oldFieldHeight; y++)
		{
			fieldBtns[y * oldFieldWidth + x]->Destroy();
		}
	}
	// Destroy top and bottom borders
	for (int x = 0; x < oldFieldWidth; x++)
	{
		topBorderBtns[x]->Destroy();
		bottomBorderBtns[x]->Destroy();
	}
	// Destory left and right borders
	for (int y = 0; y < oldFieldHeight + 2; y++)
	{
		leftBorderBtns[y] ->Destroy();
		rightBorderBtns[y] ->Destroy();
	}

	// Reassign old pointer locations to avoid memory leaks
	wxButton** old = fieldBtns;
	wxButton** oldTop = topBorderBtns;
	wxButton** oldBottom = bottomBorderBtns;
	wxButton** oldLeft = leftBorderBtns;
	wxButton** oldRight = rightBorderBtns;
	int* oldMaze = maze;

	fieldBtns = nullptr;
	topBorderBtns = nullptr;
	bottomBorderBtns = nullptr;
	leftBorderBtns = nullptr;
	rightBorderBtns = nullptr;
	maze = nullptr;

	// Free old maze field memory
	delete[] old;
	delete[] oldTop;
	delete[] oldBottom;
	delete[] oldLeft;
	delete[] oldRight;
	delete[] oldMaze;


	// Regenerate maze skeleton 
	initializeMaze();
	fieldBtns = new wxButton * [fieldWidth * fieldHeight];
	drawField();
	isCleared = true;
}

void Main::initializeMaze()
{
	//reset variables
	while (!m_stack.empty())
		m_stack.pop();

	maze = new int[mazeWidth * mazeHeight]; //[col][row] = (x, y)
	memset(maze, 0x00, mazeWidth * mazeHeight * sizeof(int));
	topCounter = 0;
	previousTop = 0;
	isComplete = false;

	// Inital starting point
	int x = rand() % mazeWidth;
	int y = rand() % mazeHeight;
	m_stack.push(make_pair(x, y));
	maze[y * mazeWidth + x] = CELL_VISITED;
	visitedNodes = 1;
}

void Main::drawField()
{
	for (int x = 0; x < fieldWidth; x++)
	{
		for (int y = 0; y < fieldHeight; y++)
		{
			fieldBtns[y * fieldWidth + x] = new wxButton(this, 10000 + (y * fieldWidth + x), "", wxPoint((MAZE_STARTING_WIDTH + 25) + 25 * x, (MAZE_STARTING_HEIGHT + 25) + 25 * y), wxSize(25, 25));
			fieldBtns[y * fieldWidth + x]->SetBackgroundColour(*wxBLACK);
		}
	}

	// Draw top and bottom borders
	topBorderBtns = new wxButton * [fieldWidth];
	bottomBorderBtns = new wxButton * [fieldWidth];
	for (int x = 0; x < fieldWidth; x++)
	{
		if (x == 0)
		{
			topBorderBtns[x] = new wxButton(this, wxID_ANY, "", wxPoint((MAZE_STARTING_WIDTH + 25) + 25 * x, MAZE_STARTING_HEIGHT), wxSize(25, 25));
			topBorderBtns[x]->SetBackgroundColour(*wxRED);
			bottomBorderBtns[x] = new wxButton(this, wxID_ANY, "", wxPoint((MAZE_STARTING_WIDTH + 25) + 25 * x, 125 + 25 * fieldHeight), wxSize(25, 25));
			bottomBorderBtns[x]->SetBackgroundColour(*wxBLACK);
		}
		else if (x == fieldWidth - 2)
		{
			topBorderBtns[x] = new wxButton(this, wxID_ANY, "", wxPoint((MAZE_STARTING_WIDTH + 25) + 25 * x, MAZE_STARTING_HEIGHT), wxSize(25, 25));
			topBorderBtns[x]->SetBackgroundColour(*wxBLACK);
			bottomBorderBtns[x] = new wxButton(this, wxID_ANY, "", wxPoint((MAZE_STARTING_WIDTH + 25) + 25 * x, 125 + 25 * fieldHeight), wxSize(25, 25));
			bottomBorderBtns[x]->SetBackgroundColour(*wxRED);
		}
		else
		{
			topBorderBtns[x] = new wxButton(this, wxID_ANY, "", wxPoint((MAZE_STARTING_WIDTH + 25) + 25 * x, MAZE_STARTING_HEIGHT), wxSize(25, 25));
			topBorderBtns[x]->SetBackgroundColour(*wxBLACK);
			bottomBorderBtns[x] = new wxButton(this, wxID_ANY, "", wxPoint((MAZE_STARTING_WIDTH + 25) + 25 * x, 125 + 25 * fieldHeight), wxSize(25, 25));
			bottomBorderBtns[x]->SetBackgroundColour(*wxBLACK);
		}
	}
	// Draw left and right borders
	leftBorderBtns = new wxButton * [fieldHeight + 2];
	rightBorderBtns = new wxButton * [fieldHeight + 2];
	for (int y = 0; y < fieldHeight + 2; y++)
	{
		leftBorderBtns[y] = new wxButton(this, wxID_ANY, "", wxPoint(MAZE_STARTING_WIDTH, MAZE_STARTING_HEIGHT + 25 * y), wxSize(25, 25));
		leftBorderBtns[y]->SetBackgroundColour(*wxBLACK);
		rightBorderBtns[y] = new wxButton(this, wxID_ANY, "", wxPoint(MAZE_STARTING_WIDTH + 25 + 25 * fieldWidth, MAZE_STARTING_HEIGHT + 25 * y), wxSize(25, 25));
		rightBorderBtns[y]->SetBackgroundColour(*wxBLACK);
	}
}

void Main::generateMaze()
{

	// lambda function to calculate index
	auto offset = [&](int x, int y)
	{
		return (m_stack.top().second + y) * mazeWidth + (m_stack.top().first + x);
	};

	// Begin maze gen algorithm	
		if (visitedNodes < mazeWidth * mazeHeight)
		{

			// Create a set of unvisted neighbours
			vector<int> neighbours;

			// North neighbour
			if (m_stack.top().second > 0 && (maze[offset(0, -1)] & CELL_VISITED) == 0)
				neighbours.push_back(0);
			// East neighbour
			if (m_stack.top().first < mazeWidth - 1 && (maze[offset(1, 0)] & CELL_VISITED) == 0)
				neighbours.push_back(1);
			// South neighbour
			if (m_stack.top().second < mazeHeight - 1 && (maze[offset(0, 1)] & CELL_VISITED) == 0)
				neighbours.push_back(2);
			// West neighbour
			if (m_stack.top().first > 0 && (maze[offset(-1, 0)] & CELL_VISITED) == 0)
				neighbours.push_back(3);

			// Are there any neighbours available?
			if (!neighbours.empty())
			{
				// Choose one available neighbour at random
				int next_cell_dir = neighbours[rand() % neighbours.size()];

				// Create a path between the neighbour and the current cell
				switch (next_cell_dir)
				{
				case 0: // North
					maze[offset(0, -1)] |= CELL_VISITED | CELL_PATH_S;
					maze[offset(0, 0)] |= CELL_PATH_N;
					m_stack.push(make_pair((m_stack.top().first + 0), (m_stack.top().second - 1)));
					break;

				case 1: // East
					maze[offset(+1, 0)] |= CELL_VISITED | CELL_PATH_W;
					maze[offset(0, 0)] |= CELL_PATH_E;
					m_stack.push(make_pair((m_stack.top().first + 1), (m_stack.top().second + 0)));
					break;

				case 2: // South
					maze[offset(0, +1)] |= CELL_VISITED | CELL_PATH_N;
					maze[offset(0, 0)] |= CELL_PATH_S;
					m_stack.push(make_pair((m_stack.top().first + 0), (m_stack.top().second + 1)));
					break;

				case 3: // West
					maze[offset(-1, 0)] |= CELL_VISITED | CELL_PATH_E;
					maze[offset(0, 0)] |= CELL_PATH_W;
					m_stack.push(make_pair((m_stack.top().first - 1), (m_stack.top().second + 0)));
					break;
				}

				visitedNodes++;
			}
			else
			{
				// No available neighbours so backtrack!
				m_stack.pop();
			}

		}

		// Clear Maze 
		for (int x = 0; x < fieldWidth; x++)
		{
			for (int y = 0; y < fieldHeight; y++)
			{
				fieldBtns[y * fieldWidth + x]->SetBackgroundColour(*wxBLACK);
			}
		}

		// Draw Maze
		for (int x = 0; x < mazeWidth; x++)
		{
			for (int y = 0; y < mazeWidth; y++)
			{
				// Each cell is inflated by m_nPathWidth, so fill it in
				for (int py = 0; py < m_nPathWidth; py++)
					for (int px = 0; px < m_nPathWidth; px++)
					{
						if (maze[y * mazeWidth + x] & CELL_VISITED)
							drawToField(x * (m_nPathWidth + 1) + px, y * (m_nPathWidth + 1) + py, 1); // Draw Cell
						else
							drawToField(x * (m_nPathWidth + 1) + px, y * (m_nPathWidth + 1) + py, 0); // Draw Cell
					}

				// Draw passageways between cells
				for (int p = 0; p < m_nPathWidth; p++)
				{
					if (maze[y * mazeWidth + x] & CELL_PATH_S)
						drawToField(x * (m_nPathWidth + 1) + p, y * (m_nPathWidth + 1) + m_nPathWidth, 1); // Draw South Passage

					if (maze[y * mazeWidth + x] & CELL_PATH_E)
						drawToField(x * (m_nPathWidth + 1) + m_nPathWidth, y * (m_nPathWidth + 1) + p, 1); // Draw East Passage
				}
			}


		}

		// Draw Unit - the top of the stack
		for (int py = 0; py < m_nPathWidth; py++)
			for (int px = 0; px < m_nPathWidth; px++)
				drawToField(m_stack.top().first * (m_nPathWidth + 1) + px, m_stack.top().second * (m_nPathWidth + 1) + py, 2);
}


void Main::drawToField(int x, int y, int col)
{
	if (col == 1 && x >= 0 && x < fieldWidth && y >= 0 && y < fieldHeight)
	{
		fieldBtns[y * fieldWidth + x]->SetBackgroundColour(*wxWHITE);
		fieldBtns[y * fieldWidth + x]->SetForegroundColour(*wxWHITE);
	}
	else if (col == 0 && x >= 0 && x < fieldWidth && y >= 0 && y < fieldHeight)
	{
		fieldBtns[y * fieldWidth + x]->SetBackgroundColour(*wxBLACK);
		fieldBtns[y * fieldWidth + x]->SetForegroundColour(*wxBLACK);
	}
	else if (col == 2 && x >= 0 && x < fieldWidth && y >= 0 && y < fieldHeight)
	{
		fieldBtns[y * fieldWidth + x]->SetBackgroundColour(*wxGREEN);
		fieldBtns[y * fieldWidth + x]->SetForegroundColour(*wxGREEN);
		if (previousTop == y * fieldWidth + x)
			topCounter++;
		else
			topCounter = 0;
		if (topCounter >= mazeHeight)
		{
			isComplete = true;
			fieldBtns[y * fieldWidth + x]->SetBackgroundColour(*wxWHITE);
			fieldBtns[y * fieldWidth + x]->SetForegroundColour(*wxWHITE);
		}
		previousTop = y * fieldWidth + x;
	}
}