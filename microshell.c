#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

typedef struct node
{
	char type;
	char **arg;
	struct node *left;
	struct node *right;
}	t_node;

int ft_strlen(char *str)
{
	int i = 0;
	
	if (str == NULL)
		return (0);
	while(str[i])
		i++;
	return (i);
}

void	print_ast(t_node *node)
{
	if (node->left != NULL)
		print_ast(node->left);
	if (node->right != NULL)
		print_ast(node->right);
	if (node->arg != NULL)
	{
		for(int i=0;node->arg[i];i++)
			printf("[%s] ", node->arg[i]);
			printf("\n");
	}
}

t_node *create_node(int type, char ***av, t_node *left, t_node *right)
{
	t_node *node;
	int	i = 0;

	if ((node = malloc(sizeof(t_node))) == NULL)
		return (NULL);	
	node->type = type;
	node->arg = NULL;
	node->left = left;
	node->right = right;
	if (type == 'c')
	{
		while ((*av)[i] && strcmp((*av)[i], "|") && strcmp((*av)[i], ";"))
			i++;	
		if ((node->arg = malloc(sizeof(char *) * (i + 1))) == NULL)
			return (NULL);
		node->arg[i] = 0;
		i = 0;
		while ((*av)[i] && strcmp((*av)[i], "|")&& strcmp((*av)[i], ";"))
		{
			node->arg[i] = (*av)[i];
			i++;
		}
	}
	*av += i;
	return (node);
}

t_node *get_tree(char ***av)
{
	t_node *ast;
	int	i = 0;

	ast = create_node('c', av, NULL, NULL);
	while((*av)[i] && !strcmp((*av)[i], "|"))
	{
		(*av)++;
		ast = create_node('p', av, ast, create_node('c', av, NULL, NULL));
	}
	return (ast);
}

int	exec_cmd(t_node *ast, char **env)
{
	pid_t pid = fork();

	if (pid > 0)
		waitpid(pid, NULL, 0); 
	else if (pid == 0)
		execve(ast->arg[0], ast->arg, env);
	return (0);
}

int exec_ast(t_node *ast, char **env)
{
	int pfd[2];
	int	status;
	pipe(pfd);
	pid_t pid = fork();

	/*if (ast->left != NULL && ast->right != NULL)
	{
		if (pid == 0)
		{
			close(pfd[1]);
			dup2(pfd[0], 0);
			close(pfd[0]);	
			exec_ast(ast->left, env);
			exit(0);
		}
		else if (pid > 0)
		{
			close(pfd[0]);
			dup2(pfd[1], 1);
			close(pfd[1]);
			exec_ast(ast->right, env);
			waitpid(pid, &status, 0);
		}
	}
	*/if (ast->type == 'c')
		exec_cmd(ast, env);
	return (0);
}

int main(int ac, char **av, char **env)
{
	t_node *ast;

	while (*av) 
	{
		av++;	
		ast = get_tree(&av);
		exec_ast(ast, env);
	}
}

