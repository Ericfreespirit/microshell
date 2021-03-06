#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void	after_main() __attribute__((destructor));


void	after_main()
{
	system("leaks a.out");
}

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

void ft_error(char *s1, char *s2)
{
	if (s1)
		write(2, s1, ft_strlen(s1));
	if (s2)
		write(2, s2, ft_strlen(s2));
	write(1, "\n", 1);
	exit(1);
}

void	print_ast(t_node *node)
{
	if (node->left != NULL)
		print_ast(node->left);
	if (node->right != NULL)
		print_ast(node->right);
	printf("type: %c\n", node->type);
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
		ft_error("error: fatal\n", NULL);	
	node->type = type;
	node->arg = NULL;
	node->left = left;
	node->right = right;
	if (type == 'c')
	{
		while ((*av)[i] && strcmp((*av)[i], "|") && strcmp((*av)[i], ";"))
			i++;	
		if ((node->arg = malloc(sizeof(char *) * (i + 1))) == NULL)
			ft_error("error: fatal\n", NULL);	
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

void	ft_cd(t_node *ast)
{
	int i = 0;

	if (ast->arg[i])
		i++;
	if (i != 1)	
		ft_error("error: cd: bad arguments", NULL);
	else if (chdir(ast->arg[1]) == -1)
		ft_error("error: cd : cannot change directory to", ast->arg[1]);
}

int	exec_cmd(t_node *ast, char **env)
{
	pid_t pid;

	if (!strcmp(ast->arg[0], "cd"))
	{
		ft_cd(ast);
		return (0);
	}
	pid = fork();
	if (pid > 0)
		waitpid(pid, NULL, 0); 
	else if (pid == 0)
	{
		execve(ast->arg[0], ast->arg, env);
		ft_error("error: cannot execute ", ast->arg[0]);
	}
	else
		ft_error("error: fatal\n", NULL);	
	return (0);
}

int exec_ast(t_node *ast, char **env)
{
	int pfd[2];
	int	status;
	
	if (ast->type == 'c')
		exec_cmd(ast, env);
	else
	{	
		pipe(pfd);
		pid_t pid = fork();
		if (pid == 0)
		{
			close(pfd[0]);
			dup2(pfd[1], 1);
			close(pfd[1]);	
			exec_ast(ast->left, env);
			exit(0);
		}
		else if (pid > 0)
		{
			close(pfd[1]);
			dup2(pfd[0], 0);
			close(pfd[0]);
			exec_ast(ast->right, env);
			waitpid(pid, &status, 0);
		}
		else
			ft_error("error: fatal\n", NULL);	
	}
	return (0);
}

void	free_ast(t_node *node)
{
	if (node->left != NULL)
		free_ast(node->left);
	if (node->right != NULL)
		free_ast(node->right);
	if (node->arg != NULL)
		free(node->arg);
	free(node);
}

int main(int ac, char **av, char **env)
{
	t_node *ast;
	
	if (ac == 1)
		return (0);
	av++;
	while (*av) 
	{
		ast = get_tree(&av);
		exec_ast(ast, env);
		free_ast(ast);
		if (*av != NULL)
			av++;
	}
}

