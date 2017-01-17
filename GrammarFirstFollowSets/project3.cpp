/**
 *  Tarek Salama - CSE 340 P.3 - Doupe
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

extern "C" {
#include "lexer.h"
}

using namespace std;

// Rule Class
class Rule {
public:
	string nonTerm;
	vector<vector<string> > rhs;
	vector<string> firstSet;
	vector<string> followSet;

	Rule(string nonTerm) {
		this->nonTerm = nonTerm;
	}

	void addToRHS(vector<string> rhs) {
		(this->rhs).push_back(rhs);
	}
};
vector<Rule> rules;

// Global terminal and non-terminal vectors
vector<string> terms;
vector<string> nonTerms;

// Parse Input Grammar
void parse() {
	getToken();

	while (t_type != HASH) {
		rules.push_back(*new Rule(string(current_token)));
		nonTerms.push_back(current_token);

		getToken();
	}

	getToken();

	while (t_type != DOUBLEHASH) {
		while (t_type != HASH && t_type != DOUBLEHASH) {
			for (unsigned int i = 0; i < rules.size(); i++) {
				if (rules[i].nonTerm == current_token) {
					getToken();

					if (t_type == ARROW) {
						vector<string> vec;

						getToken();

						while (t_type != HASH) {
							vec.push_back(current_token);

							getToken();
						}

						vec.push_back("#");
						rules[i].addToRHS(vec);
					}
					getToken();
				}
			}
		}
	}

	// Add terminals to terminal vector (terms)
	bool termFound;
	bool nonTermFound;

	for (unsigned int i = 0; i < rules.size(); i++) {
		for (unsigned int j = 0; j < rules[i].rhs.size(); j++) {
			for (unsigned int k = 0; k < rules[i].rhs[j].size(); k++) {
				nonTermFound = false;
				termFound = false;

				for (unsigned int m = 0; m < terms.size(); m++)
					if (terms[m] == (rules[i].rhs[j][k])) {
						termFound = true;
						break;
					}

				for (unsigned int m = 0; m < nonTerms.size(); m++)
					if (nonTerms[m] == (rules[i].rhs[j][k])) {
						nonTermFound = true;
						break;
					}

				if (!termFound && !nonTermFound && rules[i].rhs[j][k] != "#")
					terms.push_back(rules[i].rhs[j][k]);
			}
		}
	}
}

int getIndex(string str) {
	int index = -1;

	for (unsigned int i = 0; i < rules.size(); i++)
		if (rules[i].nonTerm == str)
			index = i;

	return index;
}

bool isNonTerm(string str) {
	bool nonTerminal = false;

	for (unsigned int i = 0; i < nonTerms.size(); i++)
		if (nonTerms[i] == str)
			nonTerminal = true;

	return nonTerminal;
}

void calculateFirstSets() {
	bool same = false;

	vector<string>::iterator it;

	while (!same) {
		same = true;

		for (unsigned int i = 0; i < rules.size(); i++) {

			// Rules 1/3
			for (unsigned int j = 0; j < rules[i].rhs.size(); j++) {
				if (!isNonTerm(rules[i].rhs[j].front())) {
					it = find(rules[i].firstSet.begin(),
							rules[i].firstSet.end(), rules[i].rhs[j].front());

					if (it == rules[i].firstSet.end()) {
						rules[i].firstSet.push_back(rules[i].rhs[j].front());
						same = false;
					}
				} else if (isNonTerm(rules[i].rhs[j].front())) {
					int index = getIndex(rules[i].rhs[j].front());
					for (unsigned int k = 0; k < rules[index].firstSet.size();
							k++) {
						if (rules[index].firstSet[k] != "#") {
							it = find(rules[i].firstSet.begin(),
									rules[i].firstSet.end(),
									rules[index].firstSet[k]);

							if (it == rules[i].firstSet.end()) {
								rules[i].firstSet.push_back(
										rules[index].firstSet[k]);
								same = false;
							}
						}
					}
				}
			}

			// Rule 2
			for (unsigned int j = 0; j < rules[i].rhs.size(); j++) {
				if (rules[i].rhs[j].front() == "#") {
					it = find(rules[i].firstSet.begin(),
							rules[i].firstSet.end(), "#");

					if (it == rules[i].firstSet.end()) {
						rules[i].firstSet.push_back("#");
						same = false;
					}
				}
			}

			// Rules 4/5
			for (unsigned int j = 0; j < rules[i].rhs.size(); j++) {
				bool eps = true;
				for (unsigned int k = 0; k < rules[i].rhs[j].size(); k++) {
					string token = rules[i].rhs[j][k];

					if (isNonTerm(rules[i].rhs[j][k])) {
						int index = getIndex(rules[i].rhs[j][k]);

						// Check if Epsilon
						bool isEpsilon = false;

						for (unsigned int x = 0;
								x < rules[index].firstSet.size(); x++)
							if (rules[index].firstSet[x] == "#") {
								isEpsilon = true;
								break;
							}

						if (isEpsilon) {
							if (eps) {
								for (unsigned int m = 0;
										m < rules[index].firstSet.size(); m++) {
									it = find(rules[i].firstSet.begin(),
											rules[i].firstSet.end(),
											rules[index].firstSet[m]);

									if (it == rules[i].firstSet.end()
											&& rules[index].firstSet[m]
													!= "#") {
										rules[i].firstSet.push_back(
												rules[index].firstSet[m]);
										same = false;
									}
								}
							}
						} else
							eps = false;
					} else if (eps) {
						eps = false;
						it = find(rules[i].firstSet.begin(),
								rules[i].firstSet.end(), rules[i].rhs[j][k]);

						if (it == rules[i].firstSet.end()) {
							rules[i].firstSet.push_back(rules[i].rhs[j][k]);
							same = false;
						}
					}
				}
			}
		}
	}
}

void calculateFollowSets() {
	calculateFirstSets();

	bool same = false;
	bool starting = false;

	vector<string>::iterator it;

	while (!same) {
		same = true;

		for (unsigned int i = 0; i < rules.size(); i++) {

			// Rule 1
			if (i == 0 && !starting) {
				rules[i].followSet.push_back("$");
				starting = true;
				same = false;
			}

			// Rule 2
			for (unsigned int j = 0; j < rules[i].rhs.size(); j++) {
				if (rules[i].rhs[j].size() > 0) {
					int lastIndex = -1;

					for (unsigned int k = 0; k < rules[i].rhs[j].size(); k++)
						if (rules[i].rhs[j][k] == "#")
							lastIndex = k - 1;

					if (lastIndex > -1) {
						string last = rules[i].rhs[j][lastIndex];

						if (isNonTerm(last)) {
							int unsigned index = getIndex(
									rules[i].rhs[j][lastIndex]);

							for (unsigned int m = 0;
									m < rules[i].followSet.size(); m++) {
								it = find(rules[index].followSet.begin(),
										rules[index].followSet.end(),
										rules[i].followSet[m]);

								if (it == rules[index].followSet.end()) {

									rules[index].followSet.push_back(
											rules[i].followSet[m]);
									same = false;
								}
							}
						}
					}
				}
			}

			// Rule 3
			for (unsigned int j = 0; j < rules[i].rhs.size(); j++) {
				bool eps = true;

				for (unsigned int k = 0; k < rules[i].rhs[j].size(); k++) {
					if (isNonTerm(rules[i].rhs[j][k])) {
						if (eps) {

							// Check if rhs is epsilon
							bool rhsEpsilon = true;

							for (unsigned int x = k + 1;
									x < rules[i].rhs[j].size(); x++) {
								if (isNonTerm(rules[i].rhs[j][x])) {

									int index = getIndex(rules[i].rhs[j][x]);

									bool isEpsilon = false;

									for (unsigned int y = 0;
											y < rules[index].firstSet.size();
											y++)
										if (rules[index].firstSet[y] == "#") {
											isEpsilon = true;
											break;
										}

									if (!isEpsilon)
										rhsEpsilon = false;

								} else if (!isNonTerm(rules[i].rhs[j][x])
										&& rules[i].rhs[j][x] != "#")
									rhsEpsilon = false;
							}

							if (rhsEpsilon) {
								int index = getIndex(rules[i].rhs[j][k]);

								for (unsigned int m = 0;
										m < rules[i].followSet.size(); m++) {
									it = find(rules[index].followSet.begin(),
											rules[index].followSet.end(),
											rules[i].followSet[m]);

									if (it == rules[index].followSet.end()) {
										rules[index].followSet.push_back(
												rules[i].followSet[m]);
										same = false;
									}
								}
							}
						}
					} else
						eps = false;
				}
			}

			// Rule 4
			for (unsigned int j = 0; j < rules[i].rhs.size(); j++) {
				for (unsigned int k = 0; k < rules[i].rhs[j].size(); k++) {
					if (isNonTerm(rules[i].rhs[j][k])) {
						if ((k + 1) <= rules[i].rhs[j].size()) {

							if (isNonTerm(rules[i].rhs[j][k + 1])) {
								int cur = getIndex(rules[i].rhs[j][k]);

								int next = getIndex(rules[i].rhs[j][k + 1]);

								unsigned int nextFirstLength =
										rules[next].firstSet.size();

								for (unsigned int m = 0; m < nextFirstLength;
										m++) {
									it = find(rules[cur].followSet.begin(),
											rules[cur].followSet.end(),
											rules[next].firstSet[m]);
									if (it == rules[cur].followSet.end()
											&& rules[next].firstSet[m] != "#") {
										rules[cur].followSet.push_back(
												rules[next].firstSet[m]);
										same = false;
									}
								}
							} else if (!isNonTerm(rules[i].rhs[j][k + 1])) {
								int cur = getIndex(rules[i].rhs[j][k]);

								it = find(rules[cur].followSet.begin(),
										rules[cur].followSet.end(),
										rules[i].rhs[j][k + 1]);

								if (it == rules[cur].followSet.end()
										&& rules[i].rhs[j][k + 1] != "#") {
									rules[cur].followSet.push_back(
											rules[i].rhs[j][k + 1]);
									same = false;
								}
							}
						}
					}
				}
			}

			// Rule 5
			for (unsigned int j = 0; j < rules[i].rhs.size(); j++) {
				for (unsigned int k = 0; k < rules[i].rhs[j].size(); k++) {
					if (isNonTerm(rules[i].rhs[j][k])) {

						int rIndex = -1;

						for (unsigned int x = k + 1; x < rules[i].rhs[j].size();
								x++) {
							if (isNonTerm(rules[i].rhs[j][x])) {

								int index = getIndex(rules[i].rhs[j][x]);

								bool isEpsilon = false;

								for (unsigned int y = 0;
										y < rules[index].firstSet.size(); y++)
									if (rules[index].firstSet[y] == "#") {
										isEpsilon = true;
										break;
									}

								if (!isEpsilon)
									rIndex = x;

							} else if (!isNonTerm(rules[i].rhs[j][x]))
								rIndex = x;
						}

						if (rIndex != -1) {
							if (isNonTerm(rules[i].rhs[j][rIndex])) {
								int cur = getIndex(rules[i].rhs[j][k]);

								int next = getIndex(rules[i].rhs[j][rIndex]);

								unsigned int nextFirstSize =
										rules[next].firstSet.size();

								for (unsigned int m = 0; m < nextFirstSize;
										m++) {
									it = find(rules[cur].followSet.begin(),
											rules[cur].followSet.end(),
											rules[next].firstSet[m]);
									if (it == rules[cur].followSet.end()
											&& rules[next].firstSet[m] != "#") {
										rules[cur].followSet.push_back(
												rules[next].firstSet[m]);
										same = false;
									}
								}
							} else if (!isNonTerm(rules[i].rhs[j][rIndex])) {
								int cur = getIndex(rules[i].rhs[j][k]);

								it = find(rules[cur].followSet.begin(),
										rules[cur].followSet.end(),
										rules[i].rhs[j][rIndex]);

								if (it == rules[cur].followSet.end()
										&& rules[i].rhs[j][rIndex] != "#") {
									rules[cur].followSet.push_back(
											rules[i].rhs[j][rIndex]);
									same = false;
								}
							}
						}
					}
				}
			}
		}
	}
}

int main(int argc, char* argv[]) {
	int task;

	if (argc < 2) {
		printf("Error: missing argument\n");
		return 1;
	}

	/*
	 Note that by convention argv[0] is the name of your executable,
	 and the first argument to your program is stored in argv[1]
	 */

	task = atoi(argv[1]);

	// TODO: Read the input grammar at this point

	/*
	 Hint: You can modify and use the lexer from previous project
	 to read the input. Note that there are only 4 token types needed
	 for reading the input in this project.

	 WARNING: You will need to modify lexer.c and lexer.h to support
	 the project 3 input language.
	 */

	switch (task) {
	case 0:
		// TODO: Output information about the input grammar

		parse();

		for (unsigned int i = 0; i < nonTerms.size(); i++)
			cout << nonTerms[i] << " ";

		cout << endl;

		sort(terms.begin(), terms.end()); // Sort terminals before output

		for (unsigned int i = 0; i < terms.size(); i++) {
			int termCount = 0;

			for (unsigned int j = 0; j < rules.size(); j++) {
				for (unsigned int k = 0; k < rules[j].rhs.size(); k++) {
					for (unsigned int m = 0; m < rules[j].rhs[k].size(); m++)
						if (terms[i] == (rules[j].rhs[k][m])) {
							termCount++;
							break;
						}
				}
			}

			cout << terms[i] << ": " << termCount << endl;
		}

		break;

	case 1:
		// TODO: Calculate FIRST sets for the input grammar
		// Hint: You better do the calculation in a function and call it here!
		// TODO: Output the FIRST sets in the exact order and format required

		parse();

		calculateFirstSets();

		for (unsigned int i = 0; i < rules.size(); i++) {
			cout << "FIRST(" << rules[i].nonTerm << ") = { ";

			for (unsigned int j = 0; j < rules[i].firstSet.size(); j++) {
				sort(rules[i].firstSet.begin(), rules[i].firstSet.end());

				if (j == rules[i].firstSet.size() - 1)
					cout << rules[i].firstSet[j] + "";
				else
					cout << rules[i].firstSet[j] + ", ";
			}
			cout << " }" << endl;
		}

		break;

	case 2:
		// TODO: Calculate FIRST sets for the input grammar
		// TODO: Calculate FOLLOW sets for the input grammar
		// TODO: Output the FOLLOW sets in the exact order and format required

		parse();

		calculateFollowSets();

		for (unsigned int i = 0; i < rules.size(); i++) {
			cout << "FOLLOW(" << rules[i].nonTerm << ") = { ";

			for (unsigned int j = 0; j < rules[i].followSet.size(); j++) {
				sort(rules[i].followSet.begin(), rules[i].followSet.end());

				if (j == rules[i].followSet.size() - 1)
					cout << rules[i].followSet[j] + "";
				else
					cout << rules[i].followSet[j] + ", ";

			}
			cout << " }" << endl;
		}

		break;

	default:
		printf("Error: unrecognized task number %d\n", task);
		break;
	}

	return 0;
}
