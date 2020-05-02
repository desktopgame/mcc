#include <stdio.h>
#include <string.h>

#include "9cc.h"
LVar* locals = NULL;
static void gen_lval(Node* node) {
  if (node->kind != ND_LVAR) error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

static char* arg_register(int i) {
  switch (i) {
    case 0:
      return "rdi";
    case 1:
      return "rsi";
    case 2:
      return "rdx";
    case 3:
      return "rcx";
    case 4:
      return "r8d";
    case 5:
      return "r9d";

    default:
      break;
  }
}

static void gen1(Node* node, int addVar) {
  static int label = 0;
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  add rsp, %d\n", addVar * 8);
      printf("  ret\n");
      return;
    case ND_IF: {
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      if (node->rhs->kind != ND_ELSE) {
        int l = label++;
        printf("  je .Lend%d\n", l);
        gen(node->rhs);
        printf(".Lend%d:\n", l);
      } else {
        Node* tBody = node->rhs->lhs;
        Node* fBody = node->rhs->rhs;
        int l = label++;
        int l2 = label++;
        printf("  je .Lelse%d\n", l);
        gen(tBody);
        printf("  jmp .Lend%d\n", l2);
        printf(".Lelse%d:\n", l);
        gen(fBody);
        printf(".Lend%d:\n", l2);
      }
    }
      return;
    case ND_WHILE: {
      int l = label++;
      int l2 = label++;
      printf(".Lbegin%d:\n", l);
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend%d\n", l2);
      gen(node->rhs);
      printf("  jmp .Lbegin%d\n", l);
      printf(".Lend%d:\n", l2);
    }
      return;
    case ND_FOR: {
      int l = label++;
      int l2 = label++;
      Node* init = node->lhs;
      Node* condAndUpdateAndBody = node->rhs;
      Node* cond = condAndUpdateAndBody->lhs;
      Node* updateAndBody = condAndUpdateAndBody->rhs;
      Node* update = updateAndBody->lhs;
      Node* body = updateAndBody->rhs;
      gen(init);
      printf(".Lbegin%d:\n", l);
      gen(cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend%d\n", l2);
      gen(body);
      gen(update);
      printf("  jmp .Lbegin%d\n", l);
      printf(".Lend%d:\n", l2);
    }
      return;
    case ND_BLOCK: {
      Node* top = node;
      while (top) {
        gen(top->lhs);
        printf("  pop rax\n");
        top = top->rhs;
      }
    }
      return;
    case ND_CALL: {
      CallNode* cNode = (CallNode*)node;
      Node* arg = node;
      char name[cNode->len + 1];
      memset(name, '\0', cNode->len + 1);
      memcpy(name, cNode->name, cNode->len);
      if (!arg) {
        // 引数0個
        printf("  mov rax, 0\n");
        printf("  call %s\n", name);
        printf("  push rax\n");
      } else {
        const int MAX_ARGS = 6;
        // gcc では引数を後ろからスタックに積んでいくので、同じように実装する
        Node* args[6];
        memset(args, 0, sizeof(Node*) * MAX_ARGS);
        for (int i = 0; i < MAX_ARGS; i++) {
          if (!arg || !arg->lhs) break;
          args[i] = arg->lhs;
          arg = arg->rhs;
        }
        for (int i = 0; i < MAX_ARGS; i++) {
          if (!args[i]) break;
          gen(args[i]);
          printf("  pop rax\n");
          printf("  mov %s, rax\n", arg_register(i));
        }
        printf("  call %s\n", name);
        printf("  push rax\n");
      }
    }
      return;
    case ND_DEFFUN: {
      Node* retAndParamsNode = node->lhs;
      Node* nameAndBodyNode = node->rhs;
      ReturnTypeNode* retNode = (ReturnTypeNode*)retAndParamsNode->lhs;
      Node* paramsNode = retAndParamsNode->rhs;
      FunctionNameNode* nameNode = (FunctionNameNode*)nameAndBodyNode->lhs;
      Node* bodyNode = nameAndBodyNode->rhs;
      // 戻り値を取得
      char retType[retNode->len + 1];
      memset(retType, '\0', retNode->len + 1);
      memcpy(retType, retNode->name, retNode->len);
      // 関数名を取得
      char funcName[nameNode->len + 1];
      memset(funcName, '\0', nameNode->len + 1);
      memcpy(funcName, nameNode->name, nameNode->len);
      printf("%s:\n", funcName);
      // 引数の数を数えて、その分だけスタックを拡張
      int parameterCount = 0;
      Node* paramsIter = paramsNode;
      while (paramsIter) {
        parameterCount++;
        paramsIter = paramsIter->rhs;
      }
      printf("  sub rsp, %d\n", parameterCount * 8);
      // ステートメントに対応したコードを生成
      Node* stmtNode = bodyNode;
      while (stmtNode) {
        gen1(stmtNode->lhs, parameterCount);
        stmtNode = stmtNode->rhs;
      }
      printf("  add rsp, %d\n", parameterCount * 8);
    }
      return;
  }
  gen(node->lhs);
  gen(node->rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NEQ:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
    default:
      break;
  }
  printf("  push rax\n");
}

void gen(Node* node) { gen1(node, 0); }
