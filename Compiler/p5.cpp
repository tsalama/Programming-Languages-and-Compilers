/**
 *  Tarek Salama | CSE 340 F16 P.5
 */

#include <iostream>
#include <stdlib.h>
#include <string.h>

using namespace std;

extern "C" {
#include "compiler.h"
}

struct id_list {
	struct ValueNode* vn;
	struct id_list* next;
}*head_id_list, *tail_id_list;

struct StatementNode *head, *tail;

struct StatementNode* parse_stmt_list();

struct ValueNode* get_node(char* n) {
	struct id_list* curr = head_id_list;
	while (curr != NULL) {
		if (strcmp(curr->vn->name, n) == 0)
			return curr->vn;

		curr = curr->next;
	}

	return NULL;
}

struct StatementNode* parse_case_list() {
	struct StatementNode* sn = new StatementNode;
	struct IfStatement* ifs = new IfStatement;

	ttype = getToken();

	if (ttype == CASE) {
		ttype = getToken();

		struct ValueNode* vn = new ValueNode;
		vn->value = atoi(token);
		ifs->condition_operand1 = NULL;
		ifs->condition_operand2 = vn;

		ifs->condition_op = NOTEQUAL;

		ttype = getToken();
		ttype = getToken();

		ifs->false_branch = parse_stmt_list();

		struct StatementNode* noop = new StatementNode;
		noop->type = NOOP_STMT;
		ifs->true_branch = noop;

		sn->type = IF_STMT;
		sn->if_stmt = ifs;
		sn->next = noop;

		ttype = getToken();
		ttype = getToken();
		if (ttype == CASE || ttype == DEFAULT) {
			ungetToken();
			sn->next->next = parse_case_list();
		} else {
			ungetToken();
		}
	} else if (ttype == DEFAULT) {
		ttype = getToken();
		ttype = getToken();

		sn = parse_stmt_list();

		ttype = getToken();
	}

	return sn;
}

struct StatementNode* parse_switch_stmt() {
	ttype = getToken();
	char* temp = strdup(token);

	ttype = getToken();

	struct StatementNode* sn = parse_case_list();

	struct StatementNode* noop = new StatementNode;
	noop->type = NOOP_STMT;

	struct GotoStatement* gt = new GotoStatement;
	gt->target = noop;
	struct StatementNode* gts = new StatementNode;
	gts->type = GOTO_STMT;
	gts->goto_stmt = gt;

	struct StatementNode* curr = sn;
	struct StatementNode* curr_2 = sn;
	while (curr->next != NULL) {
		if (curr->type == IF_STMT) {
			curr->if_stmt->condition_operand1 = get_node(strdup(temp));
			curr_2 = curr->if_stmt->false_branch;
			while (curr_2->next != NULL)
				curr_2 = curr_2->next;
			curr_2->next = gts;
		}
		curr = curr->next;
	}
	curr->next = noop;

	ttype = getToken();

	return sn;
}

struct StatementNode* parse_if_stmt() {
	struct StatementNode* sn = new StatementNode;
	struct IfStatement* ifs = new IfStatement;

	ttype = getToken();
	if (ttype == ID)
		ifs->condition_operand1 = get_node(strdup(token));
	else {
		struct ValueNode* lhs = new ValueNode;
		lhs->value = atoi(token);
		ifs->condition_operand1 = lhs;
	}

	ttype = getToken();
	if (ttype == GREATER || ttype == LESS || ttype == NOTEQUAL)
		ifs->condition_op = ttype;

	ttype = getToken();
	if (ttype == ID) {
		ifs->condition_operand2 = get_node(strdup(token));
	} else {
		struct ValueNode* rhs = new ValueNode;
		rhs->value = atoi(token);
		ifs->condition_operand2 = rhs;
	}

	ttype = getToken();

	ifs->true_branch = parse_stmt_list();
	struct StatementNode* curr = new StatementNode;
	curr = ifs->true_branch;
	while (curr->next != NULL)
		curr = curr->next;

	struct StatementNode* noop = new StatementNode;
	noop->type = NOOP_STMT;
	curr->next = noop;
	ifs->false_branch = noop;

	sn->type = IF_STMT;
	sn->if_stmt = ifs;
	sn->next = noop;

	ttype = getToken();

	return sn;
}

struct StatementNode* parse_while_stmt() {
	struct IfStatement* ws = new IfStatement;
	struct StatementNode* sn = new StatementNode;

	ttype = getToken();

	if (ttype == ID)
		ws->condition_operand1 = get_node(strdup(token));
	else {
		struct ValueNode* lhs = new ValueNode;
		lhs->value = atoi(token);
		ws->condition_operand1 = lhs;
	}

	ttype = getToken();

	if (ttype == GREATER || ttype == LESS || ttype == NOTEQUAL)
		ws->condition_op = ttype;

	ttype = getToken();

	if (ttype == ID)
		ws->condition_operand2 = get_node(strdup(token));
	else {
		struct ValueNode* rhs = new ValueNode;
		rhs->value = atoi(token);
		ws->condition_operand2 = rhs;
	}

	ttype = getToken();

	struct StatementNode* sl = parse_stmt_list();
	ws->true_branch = sl;

	struct GotoStatement* temp = new GotoStatement;
	struct StatementNode* gt = new StatementNode;
	gt->type = GOTO_STMT;
	gt->goto_stmt = temp;

	struct StatementNode* curr = new StatementNode;
	curr = ws->true_branch;
	while (curr->next != NULL)
		curr = curr->next;
	curr->next = gt;

	struct StatementNode* noop = new StatementNode;
	noop->type = NOOP_STMT;
	ws->false_branch = noop;
	sn->if_stmt = ws;
	sn->type = IF_STMT;

	ttype = getToken();

	return sn;
}

struct StatementNode* parse_print_stmt() {
	struct PrintStatement* ps = new PrintStatement;
	ttype = getToken();
	ps->id = get_node(strdup(token));

	struct StatementNode* sn = new StatementNode;
	sn->type = PRINT_STMT;
	sn->print_stmt = ps;
	sn->next = NULL;

	ttype = getToken();

	return sn;
}

struct StatementNode* parse_assign_stmt() {
	ttype = getToken();

	struct ValueNode* node = get_node(strdup(token));
	struct ValueNode* op1;
	struct ValueNode* op2 = NULL;
	int op = 0;

	struct AssignmentStatement* as = new AssignmentStatement;
	struct StatementNode* sn = new StatementNode;

	ttype = getToken();
	if (ttype == EQUAL) {
		ttype = getToken();
		if (ttype == ID || NUM) {
			if (ttype == ID)
				op1 = get_node(strdup(token));
			else {
				struct ValueNode* left = new ValueNode;
				left->value = atoi(token);
				op1 = left;
			}
			ttype = getToken();

			if (ttype == SEMICOLON) {
				as->left_hand_side = node;
				as->operand1 = op1;
				as->operand2 = op2;
				as->op = op;

				sn->type = ASSIGN_STMT;
				sn->assign_stmt = as;
				sn->next = NULL;

				return sn;
			} else if (ttype == PLUS || ttype == MINUS || ttype == MULT
					|| ttype == DIV) {
				op = ttype;
				ttype = getToken();

				if (ttype == ID)
					op2 = get_node(strdup(token));
				else if (ttype == NUM) {
					struct ValueNode* right = new ValueNode;
					right->value = atoi(token);
					op2 = right;
				}

				as->left_hand_side = node;
				as->operand1 = op1;
				as->operand2 = op2;
				as->op = op;

				sn->type = ASSIGN_STMT;
				sn->assign_stmt = as;
				sn->next = NULL;
			}

			ttype = getToken();
		}
	}

	return sn;
}

struct StatementNode* parse_stmt() {
	ttype = getToken();

	if (ttype == ID) {
		ungetToken();
		struct StatementNode* as = parse_assign_stmt();
		return as;
	} else if (ttype == PRINT) {
		struct StatementNode* ps = parse_print_stmt();
		return ps;
	} else if (ttype == WHILE) {
		struct StatementNode* wh = parse_while_stmt();
		wh->next = wh->if_stmt->false_branch;

		struct StatementNode* curr = wh->if_stmt->true_branch;
		while (curr->next->type != GOTO_STMT)
			curr = curr->next;
		curr->next->goto_stmt->target = wh;

		return wh;
	} else if (ttype == IF) {
		struct StatementNode* ifs = parse_if_stmt();
		return ifs;
	} else if (ttype == SWITCH) {
		struct StatementNode* sw = parse_switch_stmt();
		return sw;
	}
	return NULL;
}

struct StatementNode* parse_stmt_list() {
	struct StatementNode* st;
	struct StatementNode* stl;

	st = parse_stmt();
	ttype = getToken();
	if (ttype == ID || ttype == PRINT || ttype == WHILE || ttype == IF
			|| ttype == SWITCH) {
		ungetToken();
		stl = parse_stmt_list();

		struct StatementNode* curr = st;
		while (curr->next != NULL)
			curr = curr->next;

		curr->next = stl;

		return st;
	} else {
		ungetToken();
		return st;
	}
}

struct StatementNode* parse_body() {
	struct StatementNode* stl;
	ttype = getToken();

	if (ttype == LBRACE) {
		stl = parse_stmt_list();

		ttype = getToken();

		if (ttype == RBRACE)
			return stl;
	}

	return NULL;
}

void parse_decl() {
	ttype = getToken();
	if (ttype == ID) {
		// Value Node
		struct ValueNode *lhs = new ValueNode;
		struct ValueNode *rhs = new ValueNode;
		lhs->name = strdup(token);
		lhs->value = 0;
		rhs->value = 0;

		struct id_list *il = new id_list;
		il->vn = lhs;
		// Add to id list
		if (tail_id_list == NULL) {
			head_id_list = il;
			tail_id_list = il;
		} else {
			tail_id_list->next = il;
			tail_id_list = tail_id_list->next;
		}

		// Assignment Statement
		struct AssignmentStatement *as = new AssignmentStatement;
		as->left_hand_side = lhs;
		as->operand1 = rhs;
		as->op = 0;
		// Add to statement list
		struct StatementNode* sn = new StatementNode;
		sn->type = ASSIGN_STMT;
		sn->assign_stmt = as;
		if (head == NULL) {
			head = sn;
			tail = sn;
		} else {
			tail->next = sn;
			tail = sn;
		}

		ttype = getToken();

		if (ttype == COMMA)
			parse_decl();
	}
}

struct StatementNode* parse_generate_intermediate_representation() {
	parse_decl();

	struct StatementNode* stl = parse_body();
	tail->next = stl;

	return head;
}
