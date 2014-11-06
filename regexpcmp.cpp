/*
 *  REGEXP EQUALITY TESTER
 *
 *  This program test equality of regular expresion defined in language L :=
 *      {E,.,+,*,0,1} where :
 *  E := [a-z]
 *  . := string concatenation
 *  + := alternative
 *  * := Kleene star
 *  0 := empty language
 *  1 := empty string
 *
 *  INPUT:
 *          c - Cardinality of alphabet E
 *          n - Number of test cases
 *          exp1 exp2 - Regular Expresions in Reversed Polish Notation
 *  
 *  OUTPUT: {YES/NO}
 *
 */

#include <cstdio>
#include <set>
#include <map>
#include <vector>
#include <algorithm>

#define BUFF_SIZE 1024

#define PLUS '+'
#define STAR '*'
#define CONCAT '.'
#define EPSILON '1'
#define EMPTY '0'

#define IS_UNDEFINED(x)                                                  \
  (!IS_LETTER(x) && !IS_SPECSIG(x) && !IS_OPERATOR(x) && !IS_UNARY(x) && \
   !IS_BINARY(x))
#define IS_LETTER(x) (('a' <= x) && (x <= 'z'))
#define IS_SPECSIG(x) ((x == '0') || (x == '1'))
#define IS_BINARY(x) (IS_OPERATOR(x) && !IS_UNARY(x))
#define IS_OPERATOR(x) ((x == PLUS) || (x == STAR) || (x == CONCAT))
#define IS_UNARY(x) (x == STAR)

#define DEBUG_LEV false

using namespace std;

typedef set<int> STI;
typedef pair<STI, STI> PAIRSS;

int alphabet;

struct State {
  State() : trans(alphabet, -1) {}
  vector<int> trans;
  set<int> epsi;
  void clear() {
    fill(trans.begin(), trans.end(), -1);
    epsi.clear();
  }
};

vector<State> st_buffer;

struct Automata : public pair<int, int> {
  Automata(int f = 0, int s = 0) : pair<int, int>(f, s) {}
  int getStartState() { return first; }
  int getAcceptingState() { return second; }
};

set<PAIRSS> VIS_G;
char buffer[BUFF_SIZE];
vector<Automata> aut_stack;
int buff_it;

// removes all the edges
void clean() {
  st_buffer.clear();
  aut_stack.clear();
  VIS_G.clear();
  buff_it = 0;
}

// getState() - returns index of next free state in st_buffer
int getState() {
  st_buffer.push_back(State());
  int olidx = buff_it++;
  return olidx;
}

// check whether string in char * exp is a correct RPN representation
// of regular expression in defined language L
bool checkCorrect(char* exp) {
  int stk_size = 0;
  for (int i = 0; exp[i]; ++i) {
    if (IS_UNDEFINED(exp[i])) return false;
    if (IS_LETTER(exp[i]) || IS_SPECSIG(exp[i])) {
      stk_size++;
      continue;
    }
    if (IS_UNARY(exp[i])) {
      if (!(stk_size > 0)) return false;
      continue;
    }
    if (stk_size > 1)
      stk_size--;
    else
      return false;
  }
  return stk_size == 1;
}

// for given state function fill visited set by executing
// depth first search in graph constructed from epsi edges
void epsi_closure(int state, STI& visited) {
  visited.insert(state);
  for (auto it : st_buffer[state].epsi)
    if (visited.find(it) == visited.end()) epsi_closure(it, visited);
}

int finalA, finalB;

// function try to find differentiating word for builded automatas
bool search(STI& A_state, STI& B_state) {
  VIS_G.insert(PAIRSS(A_state, B_state));

#if DEBUG_LEV > 0
  printf("Search : [");
  for (auto i : A_state) printf("%d ", i);
  printf("][");
  for (auto i : B_state) printf("%d ", i);
  puts("]");
#endif

  STI tmp;
  for (auto i : A_state) epsi_closure(i, tmp);
  A_state.insert(tmp.begin(), tmp.end());
  tmp.clear();
  for (auto i : B_state) epsi_closure(i, tmp);
  B_state.insert(tmp.begin(), tmp.end());

#if DEBUG_LEV > 0
  printf("After epsi closure : [");
  for (auto i : A_state) printf("%d ", i);
  printf("][");
  for (auto i : B_state) printf("%d ", i);

  puts("]");
#endif

  tmp.clear();
  if ((A_state.find(finalA) != A_state.end())xor(B_state.find(finalB) !=
                                                 B_state.end()))
    return false;
  for (int i = 0; i < alphabet; ++i) {
    STI tmpA, tmpB;

#if DEBUG_LEV > 0
    printf("Transition over %c letter : [", 'a' + i);
#endif

    for (auto it : A_state) tmpA.insert(st_buffer[it].trans[i]);
    for (auto it : B_state) tmpB.insert(st_buffer[it].trans[i]);

#if DEBUG_LEV > 0
    for (auto i : tmpA) printf("%d ", i);
    printf("][");
    for (auto i : tmpB) printf("%d ", i);
    puts("]");
#endif

    if (VIS_G.find(PAIRSS(tmpA, tmpB)) == VIS_G.end() and !search(tmpA, tmpB))
      return false;
  }
  return true;
}

void print(int state) {
  printf("[%d] :\n", state);
  for (int i = 0; i < alphabet; ++i)
    printf("\t%c : %d\n", 'a' + i, st_buffer[state].trans[i]);
  printf("\tepsi : ");
  for (auto i : st_buffer[state].epsi) printf("%d ", i);
  puts("");
}

Automata constructEmptyString() {
  int start = getState(), end = getState();
  st_buffer[start].epsi.insert(end);
  return Automata(start, end);
}

Automata constructEmptyLanguage() {
  int start = getState(), end = getState();
  return Automata(start, end);
}

Automata constructLetter(char c) {
  int start = getState(), end = getState();
  st_buffer[start].trans[c - 'a'] = end;
  return Automata(start, end);
}

Automata constructConcatenation(Automata a, Automata b) {
  st_buffer[a.getAcceptingState()].epsi.insert(b.getStartState());
  return Automata(a.getStartState(), b.getAcceptingState());
}

Automata constructAlternative(Automata a, Automata b) {
  int start = getState(), end = getState();
  st_buffer[start].epsi.insert(a.getStartState());
  st_buffer[start].epsi.insert(b.getStartState());
  st_buffer[a.getAcceptingState()].epsi.insert(end);
  st_buffer[b.getAcceptingState()].epsi.insert(end);
  return Automata(start, end);
}

Automata constructStar(Automata a) {
  int start = getState(), end = getState();
  st_buffer[start].epsi.insert(end);
  st_buffer[start].epsi.insert(a.getStartState());
  st_buffer[a.getAcceptingState()].epsi.insert(end);
  st_buffer[a.getAcceptingState()].epsi.insert(a.getStartState());
  return Automata(start, end);
}

Automata constructAutomata() {
  Automata a, b;
  for (int i = 0; buffer[i]; ++i) {
    switch (buffer[i]) {
      case PLUS:
        b = aut_stack.back();
        aut_stack.pop_back();
        a = aut_stack.back();
        aut_stack.pop_back();
        aut_stack.push_back(constructAlternative(a, b));
        break;
      case STAR:
        a = aut_stack.back();
        aut_stack.pop_back();
        aut_stack.push_back(constructStar(a));
        break;
      case CONCAT:
        b = aut_stack.back();
        aut_stack.pop_back();
        a = aut_stack.back();
        aut_stack.pop_back();
        aut_stack.push_back(constructConcatenation(a, b));
        break;
      case EPSILON:
        aut_stack.push_back(constructEmptyString());
        break;
      case EMPTY:
        aut_stack.push_back(constructEmptyLanguage());
        break;
      default:
        aut_stack.push_back(constructLetter(buffer[i]));
    }
  }
  Automata res = aut_stack.back();
  aut_stack.pop_back();
  return res;
}

int main() {
  int cases;
  scanf("%d", &alphabet);
  scanf("%d", &cases);
  while (cases--) {
    scanf("%s", buffer);
    Automata A = constructAutomata();
    scanf("%s", buffer);
    Automata B = constructAutomata();
    finalA = A.getAcceptingState();
    finalB = B.getAcceptingState();
    STI ast, bst;
    int trash = getState();
    for (int i = 0; i < buff_it; ++i)
      for (int j = 0; j < alphabet; ++j)
        if (st_buffer[i].trans[j] == -1) st_buffer[i].trans[j] = trash;

#if DEBUG_LEV > 0
    for (int i = 0; i < buff_it; ++i) print(i);
#endif

    ast.insert(A.getStartState());
    bst.insert(B.getStartState());
    puts(search(ast, bst) ? "\033[38;5;40mYES\033[39m"
                          : "\033[38;5;160mNO\033[39m");
    clean();
  }
}
