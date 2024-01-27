#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <locale>
#include <set>
#include <map>
#include <algorithm>

using namespace std;


struct DFANode
{
	DFANode() { }

	DFANode(set<int> name, char weight)
	{
		this->name = name;
		this->weight = weight;
		this->finalState = 0;
	}

	set<int> name;
	char weight;
	bool finalState;
};

struct Node
{
	Node() { }

	Node(int name, char weight)
	{
		this->name = name;
		this->weight = weight;
	}

	int name;
	char weight;

};


// concatenates two NFAs while taking care of updating the state names so that there are no duplicate state names after concatenation
void addComponent(vector<vector<Node*>>& mainGraph, vector<vector<Node*>> component, int& stateCount)
{
	vector <Node*> edges;
	vector <Node*> alreadyUpdated;
	alreadyUpdated.resize(1);
	int flag = 0;
	int updatedFlag = 0;
	int newState = stateCount + 1;


	for (int i = 0; i < component.size() + 1; i++)
	{
		for (int j = 0; j < component.size(); j++)
		{
			for (int k = 0; k < component[j].size(); k++)
			{
				updatedFlag = 0;
				if (component[j][k]->name == i)
				{
					for (int l = 0; l < alreadyUpdated.size(); l++)
					{
						if (component[j][k] == alreadyUpdated[l])
						{
							updatedFlag = 1;
						}
					}

					if (!updatedFlag)
					{
						component[j][k]->name = newState;
						alreadyUpdated.push_back(component[j][k]);
						flag = 1;
					}
				}
			}
		}

		if (flag)
		{
			++newState;
			flag = 0;
		}
	}

	for (int i = 0; i < component.size(); i++)
	{
		for (int j = 0; j < component[i].size(); j++)
		{
			edges.push_back(component[i][j]);
		}
		mainGraph.push_back(edges);
		++stateCount;
		edges.clear();
	}
}


vector<vector<Node*>> thompsonOr(vector<vector<Node*>> a, vector<vector<Node*>> b)
{
	vector<vector<Node*>> assembled;
	vector <Node*> edge;

	// start node first epsillon transition
	Node* state = new Node(1, '^');
	edge.push_back(state);
	assembled.push_back(edge);
	edge.clear();

	int stateCount = 1;

	// add first branch of or
	addComponent(assembled, a, stateCount);

	// store end node of the branch
	int terminalState1 = stateCount;

	// start node second epsillon transition
	state = new Node(++stateCount, '^');
	assembled[0].push_back(state);

	// add second branch of or
	addComponent(assembled, b, stateCount);

	// store end node of the branch
	int terminalState2 = stateCount;

	// final vertex 
	Node* finalTransition = new Node(++stateCount, '^');
	edge.push_back(finalTransition);

	// connect the two branches to the final state 
	assembled.emplace(assembled.begin() + terminalState1, edge);
	assembled.emplace(assembled.begin() + terminalState2, edge);

	return assembled;
}


vector<vector<Node*>> thompsonAnd(vector<vector<Node*>> a, vector<vector<Node*>> b)
{
	vector<vector<Node*>> assembled;
	vector <Node*> edges;

	int stateCount = 0;
	// add first input to a graph
	addComponent(assembled, a, stateCount);

	// add epsilon transition to the same graph
	Node* midEpsillonTransition = new Node(++stateCount, '^');
	edges.push_back(midEpsillonTransition);
	assembled.push_back(edges);

	// add second input to the graph
	addComponent(assembled, b, stateCount);

	return assembled;
}


vector<vector<Node*>> thompsonKleen(vector<vector<Node*>> a, char sign)
{
	// adding first epsillon state
	vector<vector<Node*>> assembled;
	vector <Node*> edges;
	Node* state = new Node(1, '^');

	edges.push_back(state);
	assembled.push_back(edges);
	edges.clear();

	int stateCount = 1;

	// storing the state at the beginning of the loop
	int loopStart = stateCount;

	// add the input state
	addComponent(assembled, a, stateCount);

	// store the state at the end of the loop
	int loopEnd = stateCount;

	Node* loopTransition = new Node(loopStart, '^');
	edges.push_back(loopTransition);
	assembled.emplace(assembled.begin() + loopEnd, edges);

	Node* finalState = new Node(++stateCount, '^');
	assembled[loopEnd].push_back(finalState);

	if (sign == '*')
		assembled[0].push_back(finalState);

	return assembled;
}

void printGraph(vector<vector<Node*>> graph)
{
	cout << "NFA: " << endl;
	for (int i = 0; i < graph.size(); i++)
	{
		for (int j = 0; j < graph[i].size(); j++)
		{
			cout << "vertex: " << i << " to:" << graph[i][j]->name << " input: " << graph[i][j]->weight << endl;
		}
	}
}


vector<vector<Node*>> createInputGraph(char input)
{
	vector<vector<Node*>> graph;
	vector<Node*> edge;

	Node* node = new Node(1, input);

	edge.push_back(node);
	graph.push_back(edge);

	return graph;
}

int precedence(char input)
{
	if (input == '|')
		return 1;
	if (input == '.')
		return 2;
	if (input == '*' || input == '+')
		return 3;
	return 0;
}


string toPostfix(string infix)
{
	stack<char> stack;
	stack.push('#');
	string postfix = "";
	string::iterator i;

	for (i = infix.begin(); i != infix.end(); i++)
	{
		// Handling characters within single quotes as operands
		if (*i == '\'')
		{
			postfix += *i;
			++i;
			postfix += *i;
			++i;
			postfix += *i;
		}
		// if char is alphabet or numerical or specific symbol add it to postfix
		else if (isalnum(char(*i)) || *i == ',' || *i == ':' || *i == '_' || *i == '$')
			postfix += *i;
		else if (*i == '(')
			stack.push('(');
		else if (*i == ')')
		{
			while (stack.top() != '#' && stack.top() != '(')
			{
				//store and pop until ( has found
				postfix += stack.top();
				stack.pop();
			}
			//remove the '(' from stack
			stack.pop();
		}
		else // operator
		{
			if (precedence(*i) > precedence(stack.top()))
				stack.push(*i);
			else
			{
				while (stack.top() != '#' && precedence(*i) <= precedence(stack.top()))
				{
					// store and pop untill higher precedence is found
					postfix += stack.top();
					stack.pop();
				}
				stack.push(*i);
			}
		}
	}
		

	while (stack.top() != '#')
	{
		postfix += stack.top();
		stack.pop();
	}

	return postfix;
	
}



vector<vector<Node*>> evaluateRegex(string infixRegex)
{
	string postfixRegex = toPostfix(infixRegex);
	string::iterator i;

	// Defining a stack of graph type.
	stack<vector<vector<Node*>>> stack;
	vector<vector<Node*>> finalNfa;

	// Traversing the expression from left to right. 
	for (i = postfixRegex.begin(); i != postfixRegex.end(); i++) {

		// if c is alphabet or number convert it to graph then push it to stack
		if (isalnum(char(*i)) || *i == ',' || *i == ':' || *i == '_' || *i == '$')
		{
			vector<vector<Node*>> temp = createInputGraph(*i);
			stack.push(temp);
		}
		else if (*i == '\'')
		{
			++i;
			vector<vector<Node*>> temp = createInputGraph(*i);
			stack.push(temp);
			++i;
		}
		// Otherwise it is an operator.
		else
		{
			// Pop element from the stack.
			vector<vector<Node*>> op1 = stack.top();
			stack.pop();
			vector<vector<Node*>> op2;

			// Use the switch case to deal with the operand accordingly.
			switch (*i) {
			case '|':
				// Pop another element from the stack. 
				op2 = stack.top();
				stack.pop();
				stack.push(thompsonOr(op2, op1));
				break;
			case '.':
				// Pop another element from the stack. 
				op2 = stack.top();
				stack.pop();
				stack.push(thompsonAnd(op2, op1));
				break;
			case '*':
				stack.push(thompsonKleen(op1, '*'));
				break;
			case '+':
				stack.push(thompsonKleen(op1, '+'));
				break;
			}
		}
	}

	// Return the element at the top of the stack which is the final NFA.
	return stack.top();
}


map<int, set<int>> getEpsilonTransitions(vector<vector<Node*>> NFA)
{
	map<int, set<int>> epsClosure;
	set<int> states;
	for (int i = 0; i < NFA.size(); i++)
	{
		// each state has an epsilon transition to itself so it is inserted in the set
		states.insert(i);
		stack<int> eps;
		for (int j = 0; j < NFA[i].size(); j++)
		{
			// inserting states with epsilon transitions
			if (NFA[i][j]->weight == '^')
			{
				// insert current epsilon transition state
				states.insert(NFA[i][j]->name);

				// see if the current epsilon transition will lead to more epsilon transitions
				eps.push(NFA[i][j]->name);
				int top = eps.top();

				if (top < NFA.size())
				{
					while (!eps.empty())
					{
						top = eps.top();
						eps.pop();

						int l = 0;
						if (top == NFA.size())
							states.insert(top);
						else
						{
							while (l < NFA[top].size() && NFA[top][l]->weight == '^')
							{
								states.insert(NFA[top][l]->name);
								eps.push(NFA[top][l]->name);
								++l;
							}
						}

					}
				}
			}
		}
		epsClosure.insert(make_pair(i, states));
		states.clear();
	}

	// inserting final state transition (only has one epsilon transition to itself)
	states.insert(NFA.size());
	epsClosure.insert(make_pair(NFA.size(), states));

	/*
	for (auto it = epsClosure.begin(); it != epsClosure.end(); it++)
	{
		cout << it->first << ": ";
		for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{
			cout << *it2 << " ";
		}
		cout << endl;
	}
	*/

	return epsClosure;
}

set<int> unionOperation(set<int> set1, set<int> set2)
{
	// allocating memory and initializing the result container set
	set<int> result;
	for (int i = 0; i < set1.size() + set2.size(); i++)
		result.insert(-1);

	set_union(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(result, end(result)));
	result.erase(-1);

	return result;
}


map<char, set<int>> getStatesUnion(vector<vector<Node*>> NFA, set<int> states)
{
	map<char, set<int>> statesUnion;
	set<int> state;
	for (auto itr = states.begin(); itr != states.end(); itr++)
	{
		int currState = *itr;

		if (currState < NFA.size())
		{
			for (int j = 0; j < NFA[currState].size(); j++)
			{
				if (NFA[currState][j]->weight != '^')
				{
					// if current state input was not already a key in the map			
					if (statesUnion.find(NFA[currState][j]->weight) == statesUnion.end())
					{
						state.insert(NFA[currState][j]->name);

						// insert it as a key with its currently associated state
						statesUnion.insert(make_pair(NFA[currState][j]->weight, state));
						state.clear();
					}
					else
					{
						set<int> set1 = { NFA[currState][j]->name };
						set<int> set2 = statesUnion[NFA[currState][j]->weight];

						set<int> result = unionOperation(set1, set2);

						statesUnion.erase(NFA[currState][j]->weight);
						statesUnion.emplace(NFA[currState][j]->weight, result);
					}
				}
			}
		}
	}

	return statesUnion;
}


map<set<int>, set<DFANode*>> NFAtoDFA(vector<vector<Node*>> NFA)
{
	map<int, set<int>> epsilonTransitions = getEpsilonTransitions(NFA);
	set <int> DFAstate = epsilonTransitions[0];

	map<set<int>, set<DFANode*>> DFA;
	set<DFANode*> DFAstateTransitions;

	stack<set<int>> newStates;
	newStates.push(DFAstate);

	set<set<int>> oldStates;

	while (!newStates.empty())
	{
		set<int> currState = newStates.top();
		newStates.pop();
		oldStates.insert(currState);

		map<char, set<int>> statesUnion;
		statesUnion = getStatesUnion(NFA, currState);

		for (auto it1 = statesUnion.begin(); it1 != statesUnion.end(); it1++)
		{
			set<int> result = { -1 };

			for (auto it2 = it1->second.begin(); it2 != it1->second.end(); it2++)
			{
				set<int> op2 = epsilonTransitions[*it2];
				result = unionOperation(result, op2);
			}

			DFANode* dn = new DFANode();
			dn->name = result;
			dn->weight = it1->first;
			DFAstateTransitions.insert(dn);

			// if current state transition name was not added to dfa as a state, add it to newState
			// so that we can get its dfa transitions
			if (oldStates.find(dn->name) == oldStates.end())
				newStates.push(dn->name);
		}

		DFA.insert(make_pair(currState, DFAstateTransitions));
		DFAstateTransitions.clear();
	}


	return DFA;
}

set<set<int>> getDFAFinalStates(vector<vector<Node*>> NFA, map<set<int>, set<DFANode*>> DFA)
{
	int finalStateNFA = NFA.size();
	set<set<int>> DFAfinalState;
	for (auto it = DFA.begin(); it != DFA.end(); ++it)
	{
		if (it->first.find(finalStateNFA) != it->first.end())
			DFAfinalState.insert(it->first);
	}

	return DFAfinalState;
}

void printDFA(map<set<int>, set<DFANode*>> DFA)
{
	cout << "DFA: " << endl;
	for (auto it = DFA.begin(); it != DFA.end(); ++it)
	{
		cout << "state: ";
		for (auto itr = it->first.begin(); itr != it->first.end(); itr++)
		{
			cout << *itr << " ";
		}
		cout << endl;

		for (auto it1 = it->second.begin(); it1 != it->second.end(); it1++)
		{
			cout << (*it1)->weight << ": ";

			set<int> x = (*it1)->name;
			for (auto it2 = x.begin(); it2 != x.end(); it2++)
			{
				cout << *it2 << " ";
			}
			cout << endl;
		}
	}
}

void printFinalStates(set<set<int>> finalStates)
{
	cout << "Final States: " << endl;
	for (auto it1 = finalStates.begin(); it1 != finalStates.end(); it1++)
	{
		for (auto it2 = it1->begin(); it2 != it1->end(); it2++)
		{
			cout << *it2 << " ";
		}
		cout << endl;
	}
}

map<int, set<DFANode*>> renameDFAstates(map<set<int>, set<DFANode*>> DFA, set<set<int>> finalStates)
{
	map<int, set<int>> names;
	int n = -1;
	for (auto it = DFA.begin(); it != DFA.end(); ++it)
	{
		names[++n] = it->first;
	}

	for (auto it = names.begin(); it != names.end(); it++)
	{
		for (auto it2 = DFA.begin(); it2 != DFA.end(); it2++)
		{
			for (auto it1 = it2->second.begin(); it1 != it2->second.end(); it1++)
			{
				if ((*it1)->name == it->second)
				{
					(*it1)->name.clear();
					(*it1)->name.insert(it->first);
				}
			}
		}

	}

	map<int, set<DFANode*>> newDFA;
	for (auto it = names.begin(); it != names.end(); it++)
	{
		for (auto it2 = DFA.begin(); it2 != DFA.end(); it2++)
		{
			if (it2->first == it->second)
			{
				newDFA[it->first] = it2->second;
			}
		}
	
	}

	set<int> finStates;
	for (auto it = names.begin(); it != names.end(); it++)
	{
		for (auto it2 = finalStates.begin(); it2 != finalStates.end(); it2++)
		{
			if (it->second == *it2)
			{
				finStates.insert(it->first);
			}
		}
	}

	
	for (auto it = newDFA.begin(); it != newDFA.end(); it++)
	{
		for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{

			if (finStates.find(*((*it2)->name.begin())) != finStates.end())
				(*it2)->finalState = 1;
			else
				(*it2)->finalState = 0;
		}
	}

	return newDFA;
}

void printRenamedDFA(map<int, set<DFANode*>> renamedDFA)
{
	cout << endl;
	for (auto i = renamedDFA.begin(); i != renamedDFA.end(); i++)
	{
		cout << "state: " << i->first << endl;

		for (auto it = i->second.begin(); it != i->second.end(); it++)
		{
			cout << "input: " << (*it)->weight << " ";

			set<int> x = (*it)->name;
			for (auto it2 = x.begin(); it2 != x.end(); it2++)
				cout << "to: " << *it2 << " ";
			
			if((*it)->finalState)
				cout << "final state";

			cout << endl;

		}
		cout << endl;
	}
}

map<string, string> defineTokenRules()
{
	map<string, string> tokenRules;

	//Instructions
	//Arithmetic and Logical Instructions
	
	tokenRules["add"] = "a.d.d";
	
	tokenRules["addu"] = "a.d.d.u";
	
	tokenRules["addi"] = "a.d.d.i";

	tokenRules["addiu"] = "a.d.d.i.u";
	tokenRules["and"] = "a.n.d";
	tokenRules["and"] = "a.n.d.i";
	tokenRules["div"] = "d.i.v";
	tokenRules["divu"] = "d.i.v.u";
	tokenRules["mult"] = "m.u.l.t";
	tokenRules["multu"] = "m.u.l.t.u";
	tokenRules["nor"] = "n.o.r";
	tokenRules["or"] = "o.r";
	tokenRules["ori"] = "o.r.i";
	tokenRules["sll"] = "s.l.l";
	tokenRules["sllv"] = "s.l.l.v";
	tokenRules["sra"] = "s.r.a";
	tokenRules["srav"] = "s.r.a.v";
	tokenRules["srl"] = "s.r.l";
	tokenRules["srlv"] = "s.r.l.v";
	tokenRules["sub"] = "s.u.b";
	tokenRules["subu"] = "s.u.b.u";
	tokenRules["xor"] = "x.o.r";
	tokenRules["xori"] = "x.o.r.i";
	//Branch Instructions
	tokenRules["beq"] = "b.e.q";
	tokenRules["bgtz"] = "b.g.t.z";
	tokenRules["blez"] = "b.l.e.z";
	tokenRules["bne"] = "b.n.e";
	//Jump instructions
	tokenRules["j"] = "j";
	tokenRules["jal"] = "j.a.l";
	tokenRules["jr"] = "j.r";
	//Load Instructions
	tokenRules["lb"] = "l.b";
	tokenRules["lui"] = "l.u.i";
	tokenRules["lw"] = "l.w";
	//Store Instructions
	tokenRules["sb"] = "s.b";
	tokenRules["sw"] = "s.w";
	//Data Movement Instructions
	tokenRules["mfhi"] = "m.f.h.i";
	tokenRules["mflo"] = "m.f.l.o";
	//Comparison Instructions
	tokenRules["slt"] = "s.l.t";
	tokenRules["slti"] = "s.l.t.i";
	//Exception and Interrupt Instructions
	tokenRules["trap"] = "t.r.a.p";

	//System Call
	tokenRules["syscall"] = "s.y.s.c.a.l.l";
	
	//Register
	tokenRules["register"] = "$.(a.(t|(0|1|2|3))|(v|k).(0|1)|(t|s).(0|1|2|3|4|5|6|7)|t.(8|9)|(g|s|f).p|(r.a)|(z.e.r.o))";

	tokenRules["comma"] = ',';
	tokenRules["left paren"] = "\'(\'";
	tokenRules["right paren"] = "\')\'";
	tokenRules["colon"] = ':';

	string lowcaseLetters = "(a|b|c|d|e|f|g|h|i|g|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z)";
	string uppercaseLetters = "(A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|Q|R|S|T|U|V|W|X|Y|Z)";
	string number = "(1|2|3|4|5|6|7|8|9)";

	//tokenRules["Label"] = "(" + lowcaseLetters + "|" + uppercaseLetters + "|_).(" + lowcaseLetters + "|" + uppercaseLetters + "|" + number + "|\'.\'|_)*.:"; // EX -> skip:, next:
	tokenRules["decimal"] = number + "+";          //  EX -> the offset in 28($t2) which is 28
	tokenRules["hexadecimal"] = "0.(x|X).(" + number + "|(a|b|c|d|e|f)|(A|B|C|D|E|F))+"; //EX -> 0xffff

	//Assembler Directive
	tokenRules["directives"] = "\'.\'.((d.a.t.a)|(g.l.o.b.a.l)|(b.y.t.e)|(h.a.l.f)|(w.o.r.d)|(f.l.o.a.t)|(d.o.u.b.l.e)|(a.s.c.i.i.z)|(b.y.t.e)|(s.p.a.c.e)|(w.o.r.d)|(a.s.c.i.i)|(s.p.a.c.e))";
	
	//Arthmetic operators
	tokenRules["plus"] = "\'+\'"; 
	tokenRules["minus"] = "\'-\'";
	tokenRules["divide"] = "\'/\'";
	tokenRules["multiply"] = "\'*\'";

	return tokenRules;
}


/*
string unionRegex(map<string, string> rules)
{
	string combinedRegex;

	for (const auto& rule : rules)
	{
		// Concatenate each regex with the union operator |
		combinedRegex += "(" + rule.second + ")|";
	}

	// Remove the trailing '|' if it exists
	if (!combinedRegex.empty() && combinedRegex.back() == '|')
	{
		combinedRegex.pop_back();
	}

	return combinedRegex;
}
*/

bool checkString(string input, map<int, set<DFANode*>> DFA) {
	stack<char> inputStack;
	if (input.size() == 1)
		inputStack.push(input[0]);
	else {
		for (auto i = input.end(); i != input.begin(); ) {
			--i;
			inputStack.push(*i);
		}
	}

	int currState = 0;
	int finalState = 0;
	  

	while (!inputStack.empty()) {
		int found = 0;
		char currentInput = inputStack.top();
		inputStack.pop();

		auto it = DFA.find(currState);
		if (it != DFA.end()) {
			for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
				if ((*it2)->weight == currentInput) {
					currState = *((*it2)->name.begin());
					finalState = (*it2)->finalState;
					found = 1;
					break;
				}
			}
		}

		if (found == 0)
			return false;
	}

	return finalState != 0;
}

map<string, map<int, set<DFANode*>>> tokenRulesToDFA(map<string, string> tokenRules)
{
	map<string, map<int, set<DFANode*>>> tokensDFAs;
		for (auto i = tokenRules.begin(); i != tokenRules.end(); i++)
	{
		vector<vector<Node*>> NFA = evaluateRegex(i->second);
		map<set<int>, set<DFANode*>> DFA = NFAtoDFA(NFA);
		set<set<int>> finalStates = getDFAFinalStates(NFA, DFA);
		map<int, set<DFANode*>> renamedDFA = renameDFAstates(DFA, finalStates);
		tokensDFAs[i->first] = renamedDFA;
	}

	return tokensDFAs;
}

void removeSubstring(string& str, string& substr) 
{
	size_t pos = str.find(substr);

	if (pos != string::npos) {
		str.erase(pos, substr.length());
	}
}


vector<pair<string, string>> scanner(string code, map<string, map<int, set<DFANode*>>> dfaTokens)
{
	code.erase(remove(code.begin(), code.end(), ' '), code.end()); // remove whitespace from code


	vector<pair<string, string>> tokensAndLexemes;

	int n = 0;
	while (code.size() != 0)
	{
		map<string, string> acceptedSubstrings;

		int acceptedFlag = 1;
		while (acceptedFlag != 0)
		{
			acceptedFlag = 0;
			string subString = code.substr(0, ++n);
			for (auto i = dfaTokens.begin(); i != dfaTokens.end(); i++)
			{
				bool accepted = checkString(subString, i->second);
				if (accepted)
				{
					acceptedSubstrings[i->first] = subString;
					acceptedFlag = 1;
					break;
				}
			}

			if (subString.size() == code.size())
				break;
		}

		string maxLexeme;
		string maxToken;
		for (auto i = acceptedSubstrings.begin(); i != acceptedSubstrings.end(); i++)
		{
			if (i->second.size() > maxLexeme.size())
			{
				maxLexeme = i->second;
				maxToken = i->first;
			}
		}

		if (maxLexeme.size() != 0)
		{
			tokensAndLexemes.push_back(make_pair(maxToken, maxLexeme));

			acceptedSubstrings.clear();

			removeSubstring(code, maxLexeme);

			n = 0;
		}
	}

	return tokensAndLexemes;
}


void printTokensAndLexemes(vector<pair<string, string>> tokensAndLexemes)
{
	for (auto i = tokensAndLexemes.begin(); i != tokensAndLexemes.end(); i++)
	{
		cout << "Token: " << i->first << " - " << "Lexeme: " << i->second << endl;
	}
}

map<string, vector<vector<string>>> grammar = {
	{"Program", 
		{{"Statements"}}},

	{"Statements", 
		{{"Instruction"}, {"Statements", "Instruction"}}},

	{"Instruction", 
		{{"OPCODE", "Operands"}}},

	{"OPCODE", {
		{"add"}, {"addu"}, {"addi"}, {"addiu"}, {"and"},
		{"andi"}, {"div"}, {"divu"}, {"mult"}, {"multu"},
		{"nor"}, {"or"}, {"ori"}, {"sll"}, {"sllv"}, {"sra"},
		{"srav"}, {"srl"}, {"srlv"}, {"sub"}, {"subu"}, {"xor"},
		{"xori"}, {"beq"}, {"bgtz"}, {"blez"}, {"bne"}, {"j"}, {"jal"},
		{"jr"}, {"lb"}, {"lui"}, {"lw"}, {"sb"}, {"sw"}, {"mfhi"},
		{"mflo"}, {"slt"}, {"slti"}, {"trap"}
	}},

	{"Operands", 
		{{"Op"}, {"Op", "comma", "Op"}, {"Op", "comma", "Op", "comma", "Op"}}},

	{"Op", 
		{{"register"}, {"AddrImm"}}},

	{"AddrImm", 
		{{"Offset", "RegAddr"}, {"Offset"}}},

	{"RegAddr",	
		{{"left paren", "Register", "right paren"}}},

	{"SignNumber",
		{{"Number"}, {"-", "Number"}}},
	{"Offset", 
		{{"SignNumber"}, {"Number", "+", "Number"}, {"Number", "-", "Number"}}},

	{"Expr",
		{{"Expr", "+", "Expr"}, {"Expr", "-", "Expr"}, {"Expr", "*", "Expr"}, {"Expr", "/", "Expr"}, {"left paren", "Expr", "right paren"}, {"-", "Expr"}, {"Number"}}},

	{"Number", 
		{{"decimal"}, {"hexadecimal"}}}
};


// match a token with the expected value
bool match(vector<pair<string, string>> tokensAndLexemes, size_t& index, const string& expectedToken) {
	if (index < tokensAndLexemes.size() && tokensAndLexemes[index].first == expectedToken) {
		index++;
		return true;
	}
	return false;
}

void reportError(const string& error, size_t index) {
	cout << "Error: " << error << " at index " << index << endl;
}


struct ParseTreeNode {
	string value;
	vector<ParseTreeNode> children;
};

ParseTreeNode createNode(const string& value) {
	return ParseTreeNode{ value, vector<ParseTreeNode>() };
}


// function prototypes
bool Program(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent);
bool Statements(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent);
bool Instruction(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent);
bool Operands(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent);
bool OPCODE(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent);
bool Op(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent);
bool registerToken(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent);
bool AddrImm(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent);
bool RegAddr(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent);
bool Offset(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent);
bool SignNumber(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent);
bool Number(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent);


bool Program(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent) {
	if (Statements(tokensAndLexemes, index, parent)) {
		return true;
	}
	reportError("Parsing failed in Program", index);
	return false;
}

bool Statements(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent) {
	size_t currentIndex = index;
	ParseTreeNode statementsNode = createNode("Statements");

	while (Instruction(tokensAndLexemes, index, statementsNode)) {
		// Successfully parsed an instruction, continue parsing more
	}

	if (statementsNode.children.size() > 0) {
		// At least one statement was parsed, update the parse tree
		parent.children.push_back(statementsNode);
		return true;
	}

	index = currentIndex;
	if (index != tokensAndLexemes.size()) {
		reportError("Failed to parse Statements", index);
	}
	return false;
}

bool Instruction(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent) {
	ParseTreeNode instructionNode = createNode("Instruction");
	if (OPCODE(tokensAndLexemes, index, instructionNode)) {
		if (Operands(tokensAndLexemes, index, instructionNode)) {
			parent.children.push_back(instructionNode);
			return true;
		}
		else {
			reportError("Failed to parse Operands in Instruction", index);
		}
	}
	else {
		if (index != tokensAndLexemes.size())
			reportError("Failed to parse OPCODE in Instruction", index);
	}
	return false;
}
/////
bool OPCODE(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent) {
	if (index < tokensAndLexemes.size()) {
		string token = tokensAndLexemes[index].first;
		for (const auto& opcode : grammar["OPCODE"]) {
			if (token == opcode[0]) {
				ParseTreeNode opcodeNode = createNode("OPCODE");
				opcodeNode.children.push_back(createNode(token)); // Child node for the actual opcode
				parent.children.push_back(opcodeNode); // Parent node for OPCODE
				index++;
				return true;
			}
		}
		reportError("Unexpected token or invalid opcode", index);
	}
	return false;
}


bool Operands(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent) {
	ParseTreeNode operandsNode = createNode("Operands");
	if (Op(tokensAndLexemes, index, operandsNode)) {
		while (match(tokensAndLexemes, index, "comma")) {
			if (!Op(tokensAndLexemes, index, operandsNode)) {
				reportError("Failed to parse an operand from Operands", index);
				return false;
			}
		}
		parent.children.push_back(operandsNode);
		return true;
	}
	reportError("Failed to parse Operands", index);
	return false;
}


bool Op(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent) {
	ParseTreeNode opNode = createNode("Op");
	size_t tempIndex = index; // Create a temporary index to backtrack if needed

	if (registerToken(tokensAndLexemes, index, opNode) || AddrImm(tokensAndLexemes, index, opNode)) {
		parent.children.push_back(opNode);
		return true;
	}

	// Reset the index to the original position before attempting AddrImm
	index = tempIndex;

	reportError("Failed to parse a register", index);
	return false;
}

bool registerToken(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent) {
	if (match(tokensAndLexemes, index, "register")) {
		parent.children.push_back(createNode("register"));
		return true;
	}
	return false;
}

bool AddrImm(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent) {
	ParseTreeNode addrImmNode = createNode("AddrImm");
	if (Offset(tokensAndLexemes, index, addrImmNode)) {
		if (RegAddr(tokensAndLexemes, index, addrImmNode)) {
			parent.children.push_back(addrImmNode);
			return true;
		}
		else {
			reportError("Failed to parse Rgeister address in Address Immediate", index);
		}
	}
	else {
		reportError("Failed to parse Offset in Address Immediate", index);
	}
	return false;
}
bool RegAddr(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent) {
	ParseTreeNode regAddrNode = createNode("RegAddr");
	if (match(tokensAndLexemes, index, "left paren")) {
		if (registerToken(tokensAndLexemes, index, regAddrNode)) {
			if (match(tokensAndLexemes, index, "right paren")) {
				parent.children.push_back(regAddrNode);
				return true;
			}
			else {
				reportError("Expected right parenthesis in Register Address", index);
			}
		}
		else {
			reportError("Failed to parse a register in Register Address", index);
		}
	}
	else {
		reportError("Expected left parenthesis in Register Address", index);
	}
	return false;
}

bool Offset(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent) {
	ParseTreeNode offsetNode = createNode("Offset");
	if (SignNumber(tokensAndLexemes, index, offsetNode) || Number(tokensAndLexemes, index, offsetNode)) {
		parent.children.push_back(offsetNode);
		return true;
	}
	else {
		reportError("Failed to parse a signed number or a number in Offset", index);
	}
	return false;
}

bool SignNumber(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent) {
	ParseTreeNode signNumberNode = createNode("SignNumber");
	if (Number(tokensAndLexemes, index, signNumberNode)) {
		parent.children.push_back(signNumberNode);
		return true;
	}
	else if (match(tokensAndLexemes, index, "-")) {
		if (Number(tokensAndLexemes, index, signNumberNode)) {
			parent.children.push_back(createNode("-"));
			parent.children.push_back(signNumberNode);
			return true;
		}
	}
	return false;
}

bool Number(vector<pair<string, string>> tokensAndLexemes, size_t& index, ParseTreeNode& parent) {
	ParseTreeNode numberNode = createNode("Number");
	if (match(tokensAndLexemes, index, "decimal") || match(tokensAndLexemes, index, "hexadecimal")) {
		parent.children.push_back(numberNode);
		return true;
	}
	return false;
}


void printParseTree(const ParseTreeNode& node, vector<pair<string, string>> tokensAndLexemes, int depth = 0) 
{
	for (int i = 0; i < depth; ++i) {
		cout << "  |";
	}
	bool found = false;
	auto pair = tokensAndLexemes.begin();
	for (; pair != tokensAndLexemes.end(); pair++) {
		if (pair->first == node.value) {
			found = true;
			break;
		}
	}

	if (found)
		cout << "--" << node.value << ":" << pair->second << endl;
	else
		cout << "--" << node.value << endl;


	for (const auto& child : node.children) {
		printParseTree(child, tokensAndLexemes, depth + 1);
	}
}





int main(void)
{
	
	map<string, string> tokenRules = defineTokenRules();

	map<string, map<int, set<DFANode*>>> tokensDFAs = tokenRulesToDFA(tokenRules);
	
	string assemblyCode;
	cout << "enter assembly code: ";
	string line;
	while (getline(std::cin, line) && line != "") {
		assemblyCode += line; // Append each line to the inputText string
	}

	// the tokens and lexemes need to be stored in a way which their order is maintained
	vector<pair<string, string>> tokensAndLexemes = scanner(assemblyCode, tokensDFAs);
	cout << "--------------------Tokens and lexemes--------------------" << endl;
	printTokensAndLexemes(tokensAndLexemes);
	cout << "--------------------Parsing--------------------" << endl;

	size_t index = 0;
	ParseTreeNode root = createNode("Program");

	if (Program(tokensAndLexemes, index, root)) {
		cout << "Parsing successful!" << endl;
		cout << "--------------------Parse Tree--------------------" << endl;
		printParseTree(root, tokensAndLexemes);
	}
	else {
		cout << "Parsing failed!" << endl;
	}
}