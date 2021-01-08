#include "Main.h"
#include <stack>
#include <thread>
#include <list>

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
	btn_solve = new wxButton(this, EVT_SOLVE_MAZE, "Solve Maze", wxPoint(1050, 200), wxSize(100, 25));
	btn_solve->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Main::OnButtonClicked, this);
	
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
	delete[] nodeList;
}

//Event handler
void Main::OnButtonClicked(wxCommandEvent& evt)
{
	
	int evtID = int(evt.GetId());

	// If it is a field button, get coordinate of button in field array
	if (evtID < 20000) 
	{
		int btnX = (evtID - 10000) % fieldWidth;
		int btnY = (evtID - 10000) / fieldWidth;

		if (fieldBtns[btnY * fieldWidth + btnX]->GetBackgroundColour() == *wxBLACK)
		{
			fieldBtns[btnY * fieldWidth + btnX]->SetForegroundColour(*wxWHITE);
			fieldBtns[btnY * fieldWidth + btnX]->SetBackgroundColour(*wxWHITE);
		}
		else if (fieldBtns[btnY * fieldWidth + btnX]->GetBackgroundColour() == *wxWHITE)
		{
			fieldBtns[btnY * fieldWidth + btnX]->SetForegroundColour(*wxBLACK);
			fieldBtns[btnY * fieldWidth + btnX]->SetBackgroundColour(*wxBLACK);
		}
		else
		{
			fieldBtns[btnY * fieldWidth + btnX]->SetForegroundColour(*wxWHITE);
			fieldBtns[btnY * fieldWidth + btnX]->SetBackgroundColour(*wxWHITE);
		}

		isGenerated = true;
	}

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

		while (!isGenerated)
		{
			generateMaze();
			//this_thread::sleep_for(1ms);
			wxYield(); //refresh screen
		}
		isCleared = false;

		break;
	case EVT_SOLVE_MAZE:
		if (isGenerated)
		{
			initializeMaze();
			solveMaze();
		}
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
	isGenerated = false;

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
			fieldBtns[y * fieldWidth + x]->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Main::OnButtonClicked, this);
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
			isGenerated = true;
			fieldBtns[y * fieldWidth + x]->SetBackgroundColour(*wxWHITE);
			fieldBtns[y * fieldWidth + x]->SetForegroundColour(*wxWHITE);
		}
		previousTop = y * fieldWidth + x;
	}
}

void Main::initializeSolver(void)
{
	nodeList = new mazeNode[fieldWidth * fieldHeight];

	// initialize nodes, setting coordinates and obstacle nodes
	for (int x = 0; x < fieldWidth; x++)
	{
		for (int y = 0; y < fieldHeight; y++)
		{
			nodeList[y * fieldWidth + x].x = x;
			nodeList[y * fieldWidth + x].y = y;

			if (fieldBtns[y * fieldWidth + x]->GetForegroundColour() == *wxBLACK)
				nodeList[y * fieldWidth + x].isObstacle = true;
		}
	}

	// Create graph connections between nodes
	for (int x = 0; x < fieldWidth; x++)
	{
		for (int y = 0; y < fieldHeight; y++)
		{
			// Check edge conditions when creating connections
			if (y > 0)
				nodeList[y * fieldWidth + x].vecNeighbours.push_back(&nodeList[(y - 1) * fieldWidth + (x + 0)]);
			if (y < fieldHeight - 1)
				nodeList[y * fieldWidth + x].vecNeighbours.push_back(&nodeList[(y + 1) * fieldWidth + (x + 0)]);
			if (x > 0)
				nodeList[y * fieldWidth + x].vecNeighbours.push_back(&nodeList[(y + 0) * fieldWidth + (x - 1)]);
			if (x < fieldWidth - 1)
				nodeList[y * fieldWidth + x].vecNeighbours.push_back(&nodeList[(y + 0) * fieldWidth + (x + 1)]);
		}
	}
	startingNode = &nodeList[0];
	endingNode = &nodeList[fieldWidth * fieldHeight - 2];

	// Reset node states
	for (int x = 0; x < fieldWidth; x++)
	{
		for (int y = 0; y < fieldHeight; y++)
		{
			nodeList[y * fieldWidth + x].beenVisited = false;
			nodeList[y * fieldWidth + x].globalGoal = INFINITY;
			nodeList[y * fieldWidth + x].localGoal = INFINITY;
			nodeList[y * fieldWidth + x].parent = nullptr;
		}
	}

}

void Main::solveMaze(void)
{
	initializeSolver();

	// calculates diatance between centerpoints of two nodes
	auto distance = [](mazeNode* a, mazeNode* b) 
	{
		return sqrtf((a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y));
	};

	// heuristic is set to be same as distance - can be modified to change bias
	auto heuristic = [distance](mazeNode* a, mazeNode* b) 
	{
		return distance(a, b);
	};

	//Algorithm start


	// Setup starting conditions
	mazeNode* nodeCurrent = startingNode;
	startingNode->localGoal = 0.0f;
	startingNode->globalGoal = heuristic(startingNode, endingNode);

	// Add start node to not tested list - this will ensure it gets tested.
	// As the algorithm progresses, newly discovered nodes get added to this
	// list, and will themselves be tested later
	list<mazeNode*> listNotTestedNodes;
	listNotTestedNodes.push_back(startingNode);

	// if the not tested list contains nodes, there may be better paths
		// which have not yet been explored. However, we will also stop 
		// searching when we reach the target - there may well be better
		// paths but this one will do - it wont be the longest.
	while (!listNotTestedNodes.empty() && nodeCurrent != endingNode)// If we want to find absolutely shortest path, change last part to && nodeCurrent != nodeEnd)
	{
		// Sort Untested nodes by global goal, so lowest is first
		listNotTestedNodes.sort([](const mazeNode* lhs, const mazeNode* rhs) { return lhs->globalGoal < rhs->globalGoal; });

		// Front of listNotTestedNodes is potentially the lowest distance node. Our
			// list may also contain nodes that have been visited, so ditch these...
		while (!listNotTestedNodes.empty() && listNotTestedNodes.front()->beenVisited)
			listNotTestedNodes.pop_front();
		// ...or abort because there are no valid nodes left to test
		if (listNotTestedNodes.empty())
			break;
		nodeCurrent = listNotTestedNodes.front();
		nodeCurrent->beenVisited = true; // We only explore a node once

		// Check each of this node's neighbours...
		for (auto nodeNeighbour : nodeCurrent->vecNeighbours)
		{
			// ... and only if the neighbour is not visited and is 
			// not an obstacle, add it to NotTested List
			if (!nodeNeighbour->beenVisited && nodeNeighbour->isObstacle == 0)
				listNotTestedNodes.push_back(nodeNeighbour);

			// Calculate the neighbours potential lowest parent distance
			float fPossiblyLowerGoal = nodeCurrent->localGoal + distance(nodeCurrent, nodeNeighbour);

			// If choosing to path through this node is a lower distance than what 
			// the neighbour currently has set, update the neighbour to use this node
			// as the path source, and set its distance scores as necessary
			if (fPossiblyLowerGoal < nodeNeighbour->localGoal)
			{
				nodeNeighbour->parent = nodeCurrent;
				nodeNeighbour->localGoal = fPossiblyLowerGoal;

				// The best path length to the neighbour being tested has changed, so
				// update the neighbour's score. The heuristic is used to globally bias
				// the path algorithm, so it knows if its getting better or worse. At some
				// point the algo will realise this path is worse and abandon it, and then go
				// and search along the next best path.
				nodeNeighbour->globalGoal = nodeNeighbour->localGoal + heuristic(nodeNeighbour, endingNode);
			}
		}
	}

	for (int x = 0; x < fieldWidth; x++)
	{
		for (int y = 0; y < fieldHeight; y++)
		{
			if (nodeList[y * fieldWidth + x].beenVisited == true)
				fieldBtns[y * fieldWidth + x]->SetBackgroundColour(*wxCYAN);
				//fieldBtns[y * fieldWidth + x]->SetBackgroundColour(wxColour(169, 169, 169, 200));

		}
	}

	if (endingNode != nullptr)
	{
		mazeNode* p = endingNode;
		while (p->parent != nullptr)
		{
			fieldBtns[(p->y) * fieldWidth + (p->x)]->SetBackgroundColour(*wxRED);

			// Set next node to this node's parent
			p = p->parent;

			this_thread::sleep_for(10ms);
			wxYield();
		}
	}
	fieldBtns[0]->SetBackgroundColour(*wxRED);

	// clear memory
	mazeNode* oldNodeList = nodeList; 
	nodeList = nullptr;
	startingNode = nullptr;
	endingNode = nullptr;
	delete[] oldNodeList;
}