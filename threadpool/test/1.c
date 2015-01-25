/*
 *  The implementation of DB System.
 *  Copyright (C)  2008 - 2015 
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
 *  02110-1301  USA
 *
 *  Author e-mail: likunemail@163.com 
 *                 likunemail@gmail.com
 *  Last Modified: 01/25/2015, 11:21:37 AM
 *  Filename:      1.c  
 */


#include <stdlib.h>
#include <time.h>

const int MAX_OPERATION = 2;
enum TYPE{MINUS = 0, PLUS};

int _random(unsigned int n)
{
	if(n != 0)
	{
		return rand() % n;
	}
	else
	{
		return 0;
	}
}

void make_expression(int n)
{
	int left = _random(n);
	int operation = _random(MAX_OPERATION);
	int right = (PLUS==operation ? _random(left) : _random(n));
}

void make(int n, int max)
{
	int i = 0;
	for(i = 1; i <= n; i++)
	{
		make_expression(max);
		if(0 != i % 3)
		{
		}
		else
		{
		}
	}
}

#define NUM 100
int loop()
{
	int i,j,k;
	for(i=0;i<NUM;i++)
	{
		for(j=0;j<NUM;j++)
		{
			for(k=0;k<NUM;k++)
			{
				int sum = i+j;
			}
		}
	}

}

int main(int argc, char** argv)
{
loop();
	
	
	
	
	srand((int)time(0));

	if(argc != 3)
	{
		return 1;
	}

	make(atoi(argv[1]), atoi(argv[2]));

	return 0;
}
