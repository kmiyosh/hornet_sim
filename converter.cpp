#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <utility>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <climits>
#include <numeric>
#include <stdio.h>
#include <queue>
#include <iomanip>
#include <float.h>
#include <set>
#include <functional>
#include <stack>
#include <time.h>
#include <climits>
#include <bitset>
#include <fstream>
#include <stdlib.h>
#include <stdint.h>
using namespace std;
// configurations
#define PHASE 8 // repeating the same PC
#define PE 8    // physically parallel
#define NUMBER_OF_WAVEFRONT_PER_CU 8
#define TREE_MAX_DEPTH 128*3
unsigned long pattern[NUMBER_OF_WAVEFRONT_PER_CU][PHASE][PE][TREE_MAX_DEPTH];
int processed_tree_depth[NUMBER_OF_WAVEFRONT_PER_CU];
void output_pattern() {
	for (int wavefront = 0; wavefront < NUMBER_OF_WAVEFRONT_PER_CU; wavefront++) {
		for (int t = 0; t < TREE_MAX_DEPTH; t++) {
			bool finish_search = true;
			for (int phase = 0; phase < PHASE; phase++) {
				bool all_zero = true;
				for (int pe = 0; pe < PE; pe++) {
					if (pattern[wavefront][phase][pe][t] != 0) {
						all_zero = false;
					}
				}
				if (!all_zero) {
					finish_search = false;
					cout <<dec<< wavefront << " " << t << " " << phase;
					for (int pe = 0; pe < PE; pe++) {
						cout << hex << " 0x" << pattern[wavefront][phase][pe][t];
					}
					cout << "\n";
				}
			}
			if (finish_search) {
				break;
			}
		}
	}
	for (int wf = 0; wf < NUMBER_OF_WAVEFRONT_PER_CU; wf++) {
		for (int ph = 0; ph < PHASE; ph++) {
			for (int pe = 0; pe < PE; pe++) {
				for (int tr = 0; tr < TREE_MAX_DEPTH; tr++) {
					pattern[wf][ph][pe][tr] = 0;
				}
			}
		}
	}
}
int cnt = 0;
int counter = 0;
int main(int argc, char* argv[]) {
	ios::sync_with_stdio(false);
	cin.tie(NULL);
	const int WAVEFRONT_SIZE = PHASE * PE;
	const int NUMBER_OF_WORKITEM_PER_CU = PHASE * PE * NUMBER_OF_WAVEFRONT_PER_CU;

	string  search_start = "# search";
	string search_next = "END";

	string line;
	std::ifstream in("sample_direct.dat.log");
	std::cin.rdbuf(in.rdbuf());
	ofstream ofs("sample_kd_tree_converted.txt");
	cout.rdbuf(ofs.rdbuf());
	for (int wf = 0; wf < NUMBER_OF_WAVEFRONT_PER_CU; wf++) {
		for (int ph = 0; ph < PHASE; ph++) {
			for (int pe = 0; pe < PE; pe++) {
				for (int tr = 0; tr < TREE_MAX_DEPTH; tr++) {
					pattern[wf][ph][pe][tr] = 0;
				}
			}
		}
	}

	int initial_flag = 0;
	int wavefront_id = 0;
	int phase_id = 0;
	int pe_id = 0;
	int tree_depth = 0;
	int cnt = 0;
	int max_depth = 0;
	int count = 0;
	while (getline(cin, line)) {
		if (line[0] == '#') {
			if (line.find(search_next) != -1) {
				cnt++;
				tree_depth = 0;
				processed_tree_depth[wavefront_id] += max_depth + 1;
				max_depth = 0;
				pe_id = 0;
				phase_id = 0;
				wavefront_id = 0;
				output_pattern();
				for (int wa = 0; wa < NUMBER_OF_WAVEFRONT_PER_CU; wa++) {
					processed_tree_depth[wa] = 0;
				}
				cout << dec << -1 << " " << -1<<" "<<-1 << "\n";
				counter = 0;
			}
			if (line.find(search_start) != -1) {
				if (initial_flag == 0) {
					initial_flag = 1;
					count = 0;
				}
				else {
					tree_depth = 0;
					pe_id += 1;
					if (pe_id >= PE) {
						pe_id = 0;
						phase_id += 1;
						if (phase_id >= PHASE) {
							phase_id = 0;
							processed_tree_depth[wavefront_id] += max_depth+1;
							max_depth = 0;
							wavefront_id += 1;
							if (wavefront_id >= NUMBER_OF_WAVEFRONT_PER_CU) {
								wavefront_id = 0;
								count++;
							}
						}
					}
				}
			}
		}
		else {
			unsigned long addr = stoull(line, 0, 16);
			int now_depth = processed_tree_depth[wavefront_id] + tree_depth;
			max_depth = max(max_depth, tree_depth);
			pattern[wavefront_id][phase_id][pe_id][now_depth] = addr;
			tree_depth++;
		}
	}
finish:

	return 0;
}
