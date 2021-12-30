#include <stdio.h>
#include "ngx_rb_tree.h"
#include "stdlib.h"
#include "sys/time.h"
#include <unistd.h>

typedef struct channel{
    ngx_rbtree_key_t *key;              
    ngx_rbtree_node_t node;
    char *data;
}channel; 

channel findchannel;



void ngx_rbtree_left_rotate(ngx_rbtree_node_t **root,
    ngx_rbtree_node_t *sentinel, ngx_rbtree_node_t *node);
void ngx_rbtree_right_rotate(ngx_rbtree_node_t **root,
    ngx_rbtree_node_t *sentinel, ngx_rbtree_node_t *node);

void
ngx_rbtree_insert(ngx_rbtree_t *tree, ngx_rbtree_node_t *node)
{

    ngx_rbtree_node_t  **root, *temp, *sentinel;

    /* a binary tree insert */
    root = &tree->root;
    sentinel = tree->sentinel;

    if (*root == sentinel) {
        node->parent = NULL;
        node->left = sentinel;
        node->right = sentinel;
        ngx_rbt_black(node);
        *root = node;

        return;
    }

    //tree->insert(*root, node, sentinel);

    /* re-balance tree */

    while (node != *root && ngx_rbt_is_red(node->parent)) {

        if (node->parent == node->parent->parent->left) {
            temp = node->parent->parent->right;

            if (ngx_rbt_is_red(temp)) {
                ngx_rbt_black(node->parent);
                ngx_rbt_black(temp);
                ngx_rbt_red(node->parent->parent);
                node = node->parent->parent;

            } else {
                if (node == node->parent->right) {
                    node = node->parent;
                    ngx_rbtree_left_rotate(root, sentinel, node);
                }

                ngx_rbt_black(node->parent);
                ngx_rbt_red(node->parent->parent);
                ngx_rbtree_right_rotate(root, sentinel, node->parent->parent);
            }

        } else {
            temp = node->parent->parent->left;

            if (ngx_rbt_is_red(temp)) {
                ngx_rbt_black(node->parent);
                ngx_rbt_black(temp);
                ngx_rbt_red(node->parent->parent);
                node = node->parent->parent;

            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    ngx_rbtree_right_rotate(root, sentinel, node);
                }

                ngx_rbt_black(node->parent);
                ngx_rbt_red(node->parent->parent);
                ngx_rbtree_left_rotate(root, sentinel, node->parent->parent);
            }
        }
    }

    ngx_rbt_black(*root);
}

void
ngx_rbtree_delete(ngx_rbtree_t *tree, ngx_rbtree_node_t *node)
{
    int          red;
    ngx_rbtree_node_t  **root, *sentinel, *subst, *temp, *w;

    /* a binary tree delete */

    root = &tree->root;
    sentinel = tree->sentinel;

    if (node->left == sentinel) {
        temp = node->right;
        subst = node;

    } else if (node->right == sentinel) {
        temp = node->left;
        subst = node;

    } else {
        subst = ngx_rbtree_min(node->right, sentinel);
        temp = subst->right;
    }

    if (subst == *root) {
        *root = temp;
        ngx_rbt_black(temp);

        /* DEBUG stuff */
        node->left = NULL;
        node->right = NULL;
        node->parent = NULL;

        return;
    }

    red = ngx_rbt_is_red(subst);

    if (subst == subst->parent->left) {
        subst->parent->left = temp;

    } else {
        subst->parent->right = temp;
    }

    if (subst == node) {

        temp->parent = subst->parent;

    } else {

        if (subst->parent == node) {
            temp->parent = subst;

        } else {
            temp->parent = subst->parent;
        }

        subst->left = node->left;
        subst->right = node->right;
        subst->parent = node->parent;
        ngx_rbt_copy_color(subst, node);

        if (node == *root) {
            *root = subst;

        } else {
            if (node == node->parent->left) {
                node->parent->left = subst;
            } else {
                node->parent->right = subst;
            }
        }

        if (subst->left != sentinel) {
            subst->left->parent = subst;
        }

        if (subst->right != sentinel) {
            subst->right->parent = subst;
        }
    }

    /* DEBUG stuff */
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;

    if (red) {
        return;
    }

    /* a delete fixup */

    while (temp != *root && ngx_rbt_is_black(temp)) {

        if (temp == temp->parent->left) {
            w = temp->parent->right;

            if (ngx_rbt_is_red(w)) {
                ngx_rbt_black(w);
                ngx_rbt_red(temp->parent);
                ngx_rbtree_left_rotate(root, sentinel, temp->parent);
                w = temp->parent->right;
            }

            if (ngx_rbt_is_black(w->left) && ngx_rbt_is_black(w->right)) {
                ngx_rbt_red(w);
                temp = temp->parent;

            } else {
                if (ngx_rbt_is_black(w->right)) {
                    ngx_rbt_black(w->left);
                    ngx_rbt_red(w);
                    ngx_rbtree_right_rotate(root, sentinel, w);
                    w = temp->parent->right;
                }

                ngx_rbt_copy_color(w, temp->parent);
                ngx_rbt_black(temp->parent);
                ngx_rbt_black(w->right);
                ngx_rbtree_left_rotate(root, sentinel, temp->parent);
                temp = *root;
            }

        } else {
            w = temp->parent->left;

            if (ngx_rbt_is_red(w)) {
                ngx_rbt_black(w);
                ngx_rbt_red(temp->parent);
                ngx_rbtree_right_rotate(root, sentinel, temp->parent);
                w = temp->parent->left;
            }

            if (ngx_rbt_is_black(w->left) && ngx_rbt_is_black(w->right)) {
                ngx_rbt_red(w);
                temp = temp->parent;

            } else {
                if (ngx_rbt_is_black(w->left)) {
                    ngx_rbt_black(w->right);
                    ngx_rbt_red(w);
                    ngx_rbtree_left_rotate(root, sentinel, w);
                    w = temp->parent->left;
                }

                ngx_rbt_copy_color(w, temp->parent);
                ngx_rbt_black(temp->parent);
                ngx_rbt_black(w->left);
                ngx_rbtree_right_rotate(root, sentinel, temp->parent);
                temp = *root;
            }
        }
    }

    ngx_rbt_black(temp);
}


void
ngx_rbtree_left_rotate(ngx_rbtree_node_t **root, ngx_rbtree_node_t *sentinel,
    ngx_rbtree_node_t *node)
{
    ngx_rbtree_node_t  *temp;

    temp = node->right;
    node->right = temp->left;

    if (temp->left != sentinel) {
        temp->left->parent = node;
    }

    temp->parent = node->parent;

    if (node == *root) {
        *root = temp;

    } else if (node == node->parent->left) {
        node->parent->left = temp;

    } else {
        node->parent->right = temp;
    }

    temp->left = node;
    node->parent = temp;
}


void
ngx_rbtree_right_rotate(ngx_rbtree_node_t **root, ngx_rbtree_node_t *sentinel,
    ngx_rbtree_node_t *node)
{
    ngx_rbtree_node_t  *temp;

    temp = node->left;
    node->left = temp->right;

    if (temp->right != sentinel) {
        temp->right->parent = node;
    }

    temp->parent = node->parent;

    if (node == *root) {
        *root = temp;

    } else if (node == node->parent->right) {
        node->parent->right = temp;

    } else {
        node->parent->left = temp;
    }

    temp->right = node;
    node->parent = temp;
}
//这里打印了节点的颜色和父节点
void preOrderTraverse(ngx_rbtree_node_t *root)
{
    //printf("ok");
    if (root != NULL)
    {        
      //  printf("ok");

        if (root->parent != NULL)
        {
        //    printf("ok");

            printf("%d color: %d parent:%d\n", root->data, root->color, root->parent->data);
        }else{
            printf("%d color: %d\n", root->data, root->color);
        }        
        preOrderTraverse(root->left);
        preOrderTraverse(root->right);
    }
}

static void 
ngx_http_srs_hook_rbtree_insert_value(ngx_rbtree_node_t *temp,   ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel) {
 
    
	ngx_rbtree_node_t           **p;
    p = NULL;
    if(node == NULL) {
        return;
    }

    for ( ;; ) {

        if (node->key < temp->key) {
 
            p = &temp->left;
 
        } else if (node->key > temp->key) {
 
            p = &temp->right;
 
        } else { /* node->key == temp->key */
 
            break;  //目前的实现不支持相同的key的节点
        }
 
        if (*p == sentinel) {
            break;
        }
 
        temp = *p;
    }
 
    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;
    ngx_rbt_red(node);      //最后就是要让插入的节点是红色的（这是红黑树的性质)
	 
}

//时间戳插入红黑树
void insert_key_to_rbtree(ngx_rbtree_key_t key, struct ngx_rbtree_s *tree){
    struct channel *Node = malloc(sizeof(channel));
    channel *p = ngx_node_data(&(Node->node), struct channel, node);
    p->key = malloc(sizeof(char));
    *(p->key) = key;
    //printf("%c", (int)*(p->data));
    //printf("%c", 'a');
    printf("%ld\n", *(p->key));
    ngx_rbtree_insert(tree, &(p->node));
}


int if_timeout(ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel, ngx_rbtree_key_t now_time, ngx_rbtree_t *tree){

    if(ngx_rbtree_min(node, sentinel)->key < now_time){
        ngx_rbtree_delete(tree, ngx_rbtree_min(node, sentinel));
        free( ngx_rbtree_min(node, sentinel)->key);
        free(node);
        printf("timeout");
        return 0;
    }else{
        printf("no timeout");
        return 1;
    }
}




int main(){

    struct ngx_rbtree_node_s *sentinel = malloc(sizeof(struct ngx_rbtree_node_s));
    printf("ok");
    
    struct ngx_rbtree_s *tree = malloc(sizeof(struct ngx_rbtree_s));
    //定义的红黑树节点
    printf("ok");
    ngx_rbtree_init(tree, sentinel, ngx_http_srs_hook_rbtree_insert_value);
    //struct ngx_rbtree_node_s *root = node;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    printf("second: %ld\n", tv.tv_sec); // 秒
    insert_key_to_rbtree(1640843631, tree);
    insert_key_to_rbtree(1640843632, tree);
    insert_key_to_rbtree(1640843633, tree);
    insert_key_to_rbtree(1640843634, tree);

    if_timeout((tree)->root, sentinel, tv.tv_sec, tree); 
    

   /* ngx_rbtree_delete(tree, &(p->node));
    free(p->data);
    free(p);
    printf("%c\n", *(p->data));*/

    //ngx_rbtree_insert(tree, &(findchannel.node));
    //ngx_rbtree_insert(tree, &(findchannel.node));
    //ngx_rbtree_insert(tree, &(findchannel.node));
    
   
    //printf("前序遍历\n");
    //preOrderTraverse(tree->root);

}
