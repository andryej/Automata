/*
 *      Recursive descent parser for validating syntax and simplfy regular expressions
 *
 *      Author : Andrzej Gluszynski
 */

#include <cstdio>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <cstdlib>

using namespace std;

#define BUFF_SIZE 510010
#define ERROR -1

#define LETTER '@'
#define PLUS '+'
#define STAR '*'
#define CONCAT '.'
#define EPSILON '1'
#define EMPTY '0'
#define L_PARNTH '('
#define R_PARNTH ')'

#define IS_WHITESPACE(x) (x == ' ')
#define IS_UNDEFINED(x)                                                  \
  (!IS_LETTER(x) && !IS_SPECSIG(x) && !IS_OPERATOR(x) && !IS_UNARY(x) && \
   !IS_BINARY(x))
#define IS_LETTER(x) (('a' <= x) && (x < 'a' + alph_size))
#define IS_SPECSIG(x) ((x == '0') || (x == '1'))
#define IS_BINARY(x) (IS_OPERATOR(x) && !IS_UNARY(x))
#define IS_OPERATOR(x) ((x == PLUS) || (x == STAR) || (x == CONCAT))
#define IS_UNARY(x) (x == STAR)

struct pars_tree {
  pars_tree(char w = 0, int a = 0, int b = 0) : data(w), lnod(a), rnod(b) {}
  char data;
  int lnod;
  int rnod;
};

char buffer[BUFF_SIZE];
int header_pos;
int alph_size;
vector<pars_tree> p_nods;

int getNode(char w, int chld1 = -1, int chld2 = -1) {
  p_nods.push_back(pars_tree(w, chld1, chld2));
  return p_nods.size() - 1;
}

void clear() {
  header_pos = 0;
  p_nods.clear();
}

void readline() {
  int i = 0, k;
  while (true) {
    buffer[i] = getchar();
    if (buffer[i] == '\n' || buffer[i] == '\0') {
      buffer[i] = '\0';
      break;
    }
    i += (buffer[i] == ' ') ? 0 : 1;
  }
  i = k = 0;
  for (;;) {
    buffer[k++] = buffer[i];
    if (!buffer[i]) return;
    if (IS_UNARY(buffer[i])) {
      while (buffer[i] and IS_UNARY(buffer[i])) ++i;
      continue;
    }
    ++i;
  }
}

char accepted_sign;

bool accept(char w) {
  if (buffer[header_pos] ==
      w or(w == LETTER and IS_LETTER(buffer[header_pos]))) {
    accepted_sign = buffer[header_pos++];
    return true;
  }
  return false;
}

int par_S();
int par_L1();
int par_L2();
int par_L2();
int par_L3();
int par_L4();

int par_S() {
  int node = par_L1();
  return (accept(0)) ? node : ERROR;
}

int par_L1() {
  int node = ERROR;
  int lchld = 0, rchld = 0;
  // L1 -> L2
  if ((node = par_L2()) != ERROR)
    // L1 -> L2 (+ L2)*
    while (accept(PLUS) and(rchld = par_L2()) != ERROR) {
      lchld = node;
      node = getNode(PLUS, lchld, rchld);
      if (p_nods[lchld].data == EMPTY)
        node = rchld;
      else if (p_nods[rchld].data == EMPTY)
        node = lchld;
    }
  return (rchld == ERROR) ? rchld : node;
}

int par_L2() {
  int node = ERROR;
  int lchld, rchld;
  int pos;
  if ((node = par_L3()) != ERROR)
    for (;;) {
      pos = header_pos;
      if ((rchld = par_L3()) != ERROR) {
        lchld = node;
        char l = p_nods[lchld].data, r = p_nods[rchld].data;
        if (l != EMPTY and l != EPSILON and r != EMPTY and r != EPSILON) {
          node = getNode(CONCAT, lchld, rchld);
          continue;
        }
        if (l == EPSILON || r == EMPTY)
          node = rchld;
        else if (r == EPSILON || l == EMPTY)
          node = lchld;
      } else {
        header_pos = pos;
        return node;
      }
    }
  return node;
}

int par_L3() {
  int node = ERROR;
  bool flg = false;
  // L3 -> L4
  if ((node = par_L4()) != ERROR) {
    // L3 -> L4*
    while (accept(STAR)) flg = true;
    char c = p_nods[node].data;
    if (c != STAR and flg) {
      if (c != EMPTY and c != EPSILON) return getNode(STAR, node);
      p_nods[node].data = EPSILON;
    }
  }
  return node;
}

int par_L4() {
  int node = ERROR;
  // L4 -> [a-z] | 0 | 1
  if (accept(LETTER) or accept(EMPTY) or accept(EPSILON))
    return getNode(accepted_sign);
  // L4 -> ( L1 )
  if (accept(L_PARNTH) and(node = par_L1()) != ERROR and accept(R_PARNTH))
    return node;
  return ERROR;
}

bool op_cmp(char a, char b) {
  if (b == STAR or(b == CONCAT and a == PLUS)) return true;
  return false;
}

int print_tree(int nd) {
  bool prnth = false;
  if (p_nods[nd].lnod != ERROR) {
    char y = p_nods[nd].data, x = p_nods[p_nods[nd].lnod].data;
    prnth = (IS_OPERATOR(x) and IS_OPERATOR(y) and op_cmp(x, y));
    printf(prnth ? "(" : "");
    print_tree(p_nods[nd].lnod);
    printf(prnth ? ")" : "");
  }
  if (p_nods[nd].data != CONCAT)
    printf(((p_nods[nd].data != PLUS) ? "%c" : " %c "), p_nods[nd].data);
  if (p_nods[nd].rnod != ERROR) {
    char y = p_nods[nd].data, x = p_nods[p_nods[nd].rnod].data;
    prnth = (IS_OPERATOR(x) and IS_OPERATOR(y) and op_cmp(x, y));
    printf(prnth ? "(" : "");
    print_tree(p_nods[nd].rnod);
    printf(prnth ? ")" : "");
  }
  return 0;
}

int main() {
  int test_cases;
  scanf("%d %d\n", &alph_size, &test_cases);
  while (test_cases--) {
    readline();
    int root;
    ((root = par_S()) != ERROR) ? print_tree(root) || puts("")
                                : puts("SYNTAX ERROR");
    clear();
  }
}
