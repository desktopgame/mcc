#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "9cc.h"
static void indent(int depth) {
  for (int i = 0; i < depth; i++) {
    printf("  ");
  }
}

static void comment(int depth, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  // indent(depth);
  printf("# ");
  vprintf(fmt, ap);
  printf("\n");
  va_end(ap);
}
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
      return "r8";
    case 5:
      return "r9";

    default:
      break;
  }
}

static void gen1(Node* node, int depth, int addVar) {
  static int label = 0;
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      comment(depth, "begin local variable reference");
      gen_lval(node);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      comment(depth, "end local variable reference");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  pop rbx\n");
      printf("  pop rax\n");
      printf("  mov [rax], rbx\n");
      printf("  push rbx\n");
      return;
    case ND_RETURN:
      comment(depth, "begin return");
      gen1(node->lhs, depth + 1, addVar);
      printf("  pop rax\n");
      comment(depth, "begin restore local variable");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      comment(depth, "end restore local variable");
      // printf("  add rsp, %d\n", addVar * 8);
      printf("  ret\n");
      comment(depth, "end return");
      return;
    case ND_IF: {
      gen1(node->lhs, depth + 1, addVar);
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
      DefFuncNode* defFuncNode = (DefFuncNode*)node;
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
        if (paramsIter->lhs) {
          parameterCount++;
        }
        paramsIter = paramsIter->rhs;
      }
      if (!strcmp(funcName, "main")) {
        parameterCount = 2;
      }
      // ローカル変数を確保する前の位置を覚えておく
      comment(depth, "begin save local variable");
      printf("  push rbp\n");
      printf("  mov rbp, rsp\n");
      printf("  sub rsp, %d\n", parameterCount * 8);
      paramsIter = paramsNode;
      parameterCount = 0;
      while (paramsIter) {
        if (paramsIter->lhs) {
          parameterCount++;
          printf("  mov r10, rbp\n");
          printf("  sub r10, %d\n", parameterCount * 8);
          printf("  mov [r10], %s\n", arg_register(parameterCount - 1));
        }
        paramsIter = paramsIter->rhs;
      }
      comment(depth, "end save local variable");
      // ステートメントに対応したコードを生成
      Node* stmtNode = bodyNode;
      bool genAddRsp = true;
      while (stmtNode) {
        genAddRsp = true;
        gen1(stmtNode->lhs, depth + 1, parameterCount);
        if (stmtNode->lhs->kind == ND_RETURN) {
          genAddRsp = false;
        }
        stmtNode = stmtNode->rhs;
      }
      if (genAddRsp) {
        comment(depth, "begin restore local variable");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        comment(depth, "end restore local variable");
        // printf("  add rsp, %d\n", parameterCount * 8);
      }
    }
      return;
    case ND_ADDR: {
      gen_lval(node->lhs);
    }
      return;
    case ND_DEREF: {
      gen_lval(node->lhs);
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
    }
      return;
  }
  gen(node->lhs);
  gen(node->rhs);
  printf("  pop rbx\n");
  printf("  pop rax\n");
  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rbx\n");
      break;
    case ND_SUB:
      printf("  sub rax, rbx\n");
      break;
    case ND_MUL:
      printf("  imul rax, rbx\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rbx\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rbx\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NEQ:
      printf("  cmp rax, rbx\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rbx\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rbx\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
    default:
      break;
  }
  printf("  push rax\n");
}

void gen(Node* node) { gen1(node, 0, 0); }
