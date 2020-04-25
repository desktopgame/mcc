#include <stdio.h>

#include "9cc.h"
LVar* locals = NULL;
static void gen_lval(Node* node) {
  if (node->kind != ND_LVAR) error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen(Node* node) {
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
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
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
