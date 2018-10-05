/*------------------------------Tarea Logica Computacional-Construccion tabla de verdad-------------------------------**/
/************************************************************************************************************************
    Autores   : NICOLAS IGNACIO HONORATO DROGUETT; EDUARDO BALTRA ROJAS.
    Programa  : Construccion tabla de verdad.
	Proposito : Construir la tabla de verdad de una proposicion compleja o simple.
    Fecha     : Santiago de Chile, 07 de Mayo de 2018.

************************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
/**----------------------------------------VARIABLES GLOBALES--------------------------------------------------------**/

unsigned int csent;				//Se refiere a la cantidad de sentencias  de la proposicion.
unsigned int cvalores;			//Se refiere a la cantidad de combinaciones de la tabla. EJ: 3 sentencias => 2^3 combinaciones.

/**-------------------------------------------ESTRUCTURAS------------------------------------------------------------**/
struct Proposicion{					//Estructura que guarda los datos de una proposicion
	char *proposicion;				//String correspondiente a la proposicion
	unsigned int *valores;			//Tabla de valores de verdad de la proposicion (0: Falso, 1: Verdadero).
	struct Proposicion *izq,*der;	//Sub proposiciones, si la proposicion es un atomo, ambos son nulos.
};
/**-----------------------------------------------PROTOTIPOS---------------------------------------------------------**/
/**-------------------------------------------MANEJO DE STRING-------------------------------------------------------**/

char *procesarProposicion(char *s);
char *eliminarCaracteresInvalidos(char *s);
unsigned int esCaracterValido(char c);
void eliminarExpresionesLogicas(char *s);
void eliminarExpresionLogica(char *s,int n);
void eliminarParentesisInnecesarios(char *prop);
void eliminarNegacionesInnecesarias(char *prop);
unsigned int buscarCadena(char *s,int n,char *sub);
char *sentenciasProposicion(char *s);
void ordenar(char *s);
unsigned int finalParentesis(char *prop,unsigned int inicio);
char *subString(char *prop,unsigned int inicio,unsigned int fin);
char *charToString(char c);

/**-----------------------------------------MANEJO DE PROPOSICION-----------------------------------------------------**/

struct Proposicion *buscarProposicion(struct Proposicion **proposiciones,unsigned int cantidad,char *prop);
struct Proposicion **crearSentenciasIniciales(char *sentencias);
struct Proposicion *crearSentencia(char *p);

/**--------------------------------------------------ARBOL------------------------------------------------------------**/

struct Proposicion *construirArbol(struct Proposicion **iniciales,char *prop);
void imprimirTabla(struct Proposicion *abb);
void ordenarListaProposiciones(struct Proposicion **lista,unsigned int largo);
void listarArbol(struct Proposicion *abb,struct Proposicion ***lista,int *largo);

/**-------------------------------------------FUNCIONES DE TABLAS-----------------------------------------------------**/

void generarTabla(unsigned int *p,unsigned int *q,unsigned int **pq,char c);
unsigned int *negar(unsigned int *p);
unsigned int *im(unsigned int *p,unsigned int *q);
unsigned int *o(unsigned int *p,unsigned int *q);
unsigned int *y(unsigned int *p,unsigned int *q);

/**---------------------------------------------INFIX A POSTFIX-------------------------------------------------------**/

char *infixToPostfix(char *s);
char pop(char *stack);
void push(char *stack,char c);
char *crearStack(unsigned int l);
unsigned int stackVacio(char *stack);
int prec(char c);

/**----------------------------------------------POSTFIX A INFIX------------------------------------------------------**/

char *operar(char *p,char *q,char op);
char *postfixToInfix(char *s);

/**-----------------------------------------------FIN PROTOTIPOS------------------------------------------------------**/

int main(unsigned int argc, char** argv) {
	unsigned int i,j,k,c=0;
	char *prop;
	char *sentencias;
	struct Proposicion **iniciales;
	struct Proposicion **abb;
	system("clear");
	abb=(struct Proposicion**) calloc((argc-1),sizeof(struct Proposicion*));
	/**
	 * Se recorren todas las string almacenadas
	 * en argv, a excepcion de la primera casilla,
	 * la cual corresponde al nombre del programa.
	*/
	for(i=1;i<argc;i++){
		prop=procesarProposicion(argv[i]);
		sentencias=sentenciasProposicion(prop);
		ordenar(sentencias);
		csent= (unsigned int) strlen(sentencias);
		cvalores=(unsigned int) pow(2,csent);
		iniciales=crearSentenciasIniciales(sentencias);
		printf("\n#%d---------------------------\nProposicion: %s\n",i,prop);
		printf("Sentencias: %s\n",sentencias);
        printf("Combinaciones totales: %d\n",cvalores);
		abb[i-1]=construirArbol(iniciales,prop);
		/**
		 * Se imprimira la tabla de verdad solo en caso de
		 * que la cantidad de combinaciones de la tabla no
		 * sea muy grande.
		 * */
		if(cvalores<=32)	//La tabla de verdad solo se muestra cuando posea un maximo de 32 combinaciones.	
			imprimirTabla(abb[i-1]);
		for(j=0,k=cvalores-1;!((!abb[i-1]->valores[j] || !abb[i-1]->valores[k]) && c) && j<k;j++,k--){
			c+=abb[i-1]->valores[j]+abb[i-1]->valores[k];
		}
		/**
		 * Finalmente, se determina si la proposicion
		 * procesada es una tautologia, contradiccion 
		 * o contingencia, dependiendo del valor de c.
		 * */
		if(c==cvalores)
			printf("Es una tautologia!\n");
		else
			if(!c)
				printf("Es una contradiccion!\n");
			else
				printf("Es una contingencia!\n");
		c=0;
		free(iniciales);
	}
	printf("\n");
	free(abb);
	return 0;
}

/**----------------------------------------------------FUNCIONES------------------------------------------------------**/
/**
 * procesarProposicion:
 * 
 * Funcion que recibe la proposicion escrita
 * en latex y la procesa de modo que quede en
 * un formato valido para la funcion construirArbol.
 * 
 */
char *procesarProposicion(char *s){
	char *prop;
	unsigned int largo= (unsigned int) strlen(s);
	prop=(char *) calloc(largo,sizeof(char));
	strcpy(prop,s);
	eliminarExpresionesLogicas(prop);
	prop=eliminarCaracteresInvalidos(prop);
	eliminarParentesisInnecesarios(prop);
	eliminarNegacionesInnecesarias(prop);
	prop=eliminarCaracteresInvalidos(prop);
	return postfixToInfix(infixToPostfix(prop));	
}
/**
 * construirArbol:
 * 
 * Función recursiva que genera un arbol jerarquico en base 
 * a una proposición enviada.
 * 
 * La proposicion debe tener los parentesis correspondientes.
 * 
 * EJ: p and or r no es una proposicion valida.
 * 
 * */
struct Proposicion *construirArbol(struct Proposicion **iniciales,char *prop){
    struct Proposicion *abb=crearSentencia(prop);					//Raiz del arbol.
	unsigned int posConectivo = 0;									//Es la posicion del conectivo que une a ambas proposiciones.
	unsigned int negizq=0;											//Estado de negacion de la primera proposicion (izquierda).
	unsigned int negder=0;											//Estado de negacion de la segunda proposicion (derecha).
	unsigned int *auxIzq;											//Variable auxiliar para tabla nodo izquierdo del arbol.
	unsigned int *auxDer;											//Variable auxiliar para tabla nodo derecho del arbol.
	unsigned int negarTodo=0;										//Estado de negacion de la proposicion completa.
	unsigned int largoProp= (unsigned int) strlen(prop);		    //Largo de la cadena prop.
	unsigned int fParentesis;
    eliminarNegacionesInnecesarias(prop);
    prop=eliminarCaracteresInvalidos(prop);
	if(largoProp<2)
		return iniciales[0];
	else if(largoProp<3){
			iniciales[0]->valores=negar(iniciales[0]->valores);
			return iniciales[0];
	}
	if(prop[0]=='~' && prop[1]=='(') {                                //Se pregunta si TODA la proposicion esta negada.
		negarTodo = 1;
		prop[0] = ' ';
		prop = eliminarCaracteresInvalidos(prop);
	}
    eliminarParentesisInnecesarios(prop);
    prop=eliminarCaracteresInvalidos(prop);
	if(prop[0]=='~'){												//Se pregunta si la primera proposici�n esta negada.
		negizq=1;
		prop[0]=' ';
		prop=eliminarCaracteresInvalidos(prop);
	}
	if(prop[0]=='('){												//Caso 1: la primera proposicion es una proposicion compleja.
		fParentesis=finalParentesis(prop,0);
		posConectivo=fParentesis+1;
		abb->izq=construirArbol(iniciales,subString(prop,1,fParentesis-1));	//Se llama de nuevo a la funcion para asignar el nodo izquierdo.
		if(fParentesis+2<largoProp){
			if(prop[fParentesis+2]=='~'){							//Se pregunta si la segunda proposicion esta negada, para este caso.
				negder=1;
				prop[fParentesis+2]=' ';
				prop=eliminarCaracteresInvalidos(prop);
			}
			if(prop[fParentesis+2]=='(')
					abb->der=construirArbol(iniciales,subString(prop,fParentesis+3,finalParentesis(prop,fParentesis+2)-1));
				else
					if(isalpha(prop[fParentesis+2]))
						abb->der=buscarProposicion(iniciales,csent,charToString(prop[fParentesis+2]));
		}	
	}
	else{										//Caso 2: la primera proposicion es una proposicion simple(una sentencia).
		if(isalpha(prop[0])){
			posConectivo=1;
			abb->izq=buscarProposicion(iniciales,csent,charToString(prop[0]));
			if(2<largoProp){
				if(prop[2]=='~'){									//Se pregunta si la segunda proposicion esta negada, para este caso.	
					negder=1;
					prop[2]=' ';
					prop=eliminarCaracteresInvalidos(prop);
				}
				if(prop[2]=='(')									//Se llama de nuevo a la funcion ya que la segunda proposicion es compleja.
					abb->der=construirArbol(iniciales,subString(prop,2,finalParentesis(prop,2)));
				else
					if(isalpha(prop[2]))							//Se busca dentro de las proposiciones simples ya que la segunda proposicion es un atomo.
						abb->der=buscarProposicion(iniciales,csent,charToString(prop[2]));
			}
		}
	}
	/**
	 * Se generan la tabla de la proposicion final, en base
	 * a lo generado en los nodos izquierdos y derechos.
	 * */
	auxIzq=abb->izq->valores;
	auxDer=abb->der->valores;
	if(negizq)
		auxIzq=negar(auxIzq);
	if(negder)
		auxDer=negar(auxDer);
	generarTabla(auxIzq,auxDer,&(abb->valores),prop[posConectivo]);
	if(negarTodo)
		abb->valores=negar(abb->valores);
	return abb;
}




/**
 * imprimirTabla:
 * 
 * Funcion que imprime las tablas de todos los
 * nodos de un arbol proposicional.
 * 
 * */
void imprimirTabla(struct Proposicion *abb){
	unsigned int largo=1;
	unsigned int i=0,j;
	struct Proposicion **lista=(struct Proposicion**) calloc(largo,sizeof(struct Proposicion*));
	listarArbol(abb,&lista,&largo);
	largo--;
	ordenarListaProposiciones(lista,largo);
	for(;i<largo;i++)
		printf("%s|",lista[i]->proposicion);
	printf("\n");
	for(i=0;i<cvalores;i++){
		for(j=0;j<largo;j++){
			printf("%*s",(((int)strlen(lista[j]->proposicion)-1)/2),"");
			if(lista[j]->valores[i])
				printf("T");
			else
				printf("F");
			printf("%*s|",(((int)strlen(lista[j]->proposicion))/2),"");
		}
		printf("\n");
	}
	free(lista);
}





/**
 * listarArbol:
 * 
 * Funcion que crea una lista en in-orden con todos 
 * los nodos de un arbol proposicional.
 * 
 * */
void listarArbol(struct Proposicion *abb,struct Proposicion ***lista,int *largo){
	if(abb){
		if(*largo==1 || buscarProposicion(*lista,*largo-1,abb->proposicion)==NULL){
			(*lista)[(*largo)-1]=abb;
			*(lista)=(struct Proposicion**) realloc(*(lista),++(*largo)*sizeof(struct Proposicion*));
		}
		listarArbol(abb->izq,&(*lista),&(*largo));
		listarArbol(abb->der,&(*lista),&(*largo));
	}
}




/**
 * ordenarListaProposiciones:
 * 
 * Funcion que ordena una lista de struct Proposicion
 * por tamaño de su proposicion.
 * 
 * */
void ordenarListaProposiciones(struct Proposicion **lista,unsigned int largo){
	unsigned int i=0,j;
	struct Proposicion *aux;
	for(;i<largo-1;i++)
		for(j=i;j<largo;j++)
			if(strlen(lista[i]->proposicion)>strlen(lista[j]->proposicion) || (strlen(lista[i]->proposicion)==strlen(lista[j]->proposicion) && 0<strcmp(lista[i]->proposicion,(lista[j]->proposicion)))){
				aux=lista[i];
				lista[i]=lista[j];
				lista[j]=aux;
			}
}






/**
 * buscarProposicion:
 * 
 * Funcion que busca una proposicion en una lista
 * enviada por parametro.
 * 
 * Si no encuentra el puntero retorna NULL.
 * */
struct Proposicion *buscarProposicion(struct Proposicion **proposiciones,unsigned int cantidad,char *prop){
	unsigned int i=0;
	for(;i<cantidad;i++)
		if(proposiciones[i]!=NULL && !strcmp(proposiciones[i]->proposicion,prop))
			return proposiciones[i];
	return NULL;
}




/**
 * crearSentenciasIniciales:
 * 
 * Funcion que crea una lista con todos los
 * atomos de la proposicion.
 * */
struct Proposicion **crearSentenciasIniciales(char *sentencias){
	struct Proposicion **array;
	unsigned int i=0,j=0;
	unsigned int c1=1,c2=1,swt=0;
	array=(struct Proposicion**) calloc(csent,sizeof(struct Proposicion*));
	for(;i<csent;i++)
		array[i]=crearSentencia(charToString(sentencias[i]));
	for(i=0;i<csent;i++){
		for(j=0;j<cvalores;j++,c2++){
			if(swt%2!=0)
				array[i]->valores[j]=1;
			if(c2==c1){
				c2=0;
				swt++;
			}
		}
		swt=0;
		c1*=2;
	}
	return array;
}




/**
 * crearSentencia:
 * 
 * Funcion que crea un puntero a struct Proposicion
 * dado una proposicion p.
 * */
struct Proposicion *crearSentencia(char *p){
	struct Proposicion *sentencia;
	sentencia=(struct Proposicion*) calloc(1,sizeof(struct Proposicion));
	sentencia->der=sentencia->izq=NULL;
	sentencia->proposicion=eliminarCaracteresInvalidos(p);
	sentencia->valores=(int*) calloc(cvalores,sizeof(int));
	return sentencia;
}





/**
 * sentenciasProposicion:
 * 
 * Funcion que crea un array de caracteres
 * con todas las sentencias de la proposicion.
 * 
 * (no repetibles).
 * */
char *sentenciasProposicion(char *s){
	unsigned int c=0;
	unsigned int i=0;
	unsigned int largo=(unsigned int) strlen(s);
	char *sentencias=(char *) calloc(largo,sizeof(char));
	sentencias[0]='\0';
	for(i=0;i<largo;i++)
		if(isalpha(s[i]) && strchr(sentencias,s[i])==NULL){
			sentencias[c]=s[i];
			sentencias[++c]='\0';
		}
	return sentencias;
}





/**
 * ordenar:
 * 
 * Funcion que ordena los caracteres de una string
 * de mayor a menor.
 * */
void ordenar(char *s){
	unsigned int i=0,j;
	unsigned int largo=(unsigned int) strlen(s);
	char aux;
	for(;i<largo-1;i++)
		for(j=i;j<largo;j++)
			if(s[j]>s[i]){
				aux=s[j];	s[j]=s[i];	s[i]=aux;
			}	
}




/**
 * eliminar Espacio:
 * 
 * Funcion que elimina todos los espacios de una 
 * string.
 * */
char *eliminarCaracteresInvalidos(char *s){
	unsigned int i=0;
	int j=-1;
	unsigned int largo=(unsigned int) strlen(s);
	char *s2=(char *) calloc(largo,sizeof(char));
	for(;i<largo;i++)
		if(isalpha(s[i]) || esCaracterValido(s[i]))
			s2[++j]=s[i];
	s2[++j]='\0';
	return s2;
}

unsigned int esCaracterValido(char c){
	return c!='('&&c!=')'&&c!='+'&&c!='*'&&c!='>'&&c!='~'?0:1;
}


/**
 * eliminarExpresionesLogicas:
 * 
 * Funcion que elimina todas los operadores logicos
 * de una proposicion escrita en latex.
 * 
 * */
void eliminarExpresionesLogicas(char *s){
	unsigned int i=0;
	unsigned int largo=(unsigned int)strlen(s);
	for(;i<largo;i++)
		if(s[i]=='\\')
			eliminarExpresionLogica(s,i);
}




/**
 * eliminarExpresionLogica:
 * 
 * Funcion que elimina un operador logico desde
 * una posicion n en la string enviada.
 * 
 * Luego, la cambia a su expresion unitaria
 * correspondiente.
 * */
void eliminarExpresionLogica(char *s,int n){
	unsigned int i=0;
	if(s[n+1]=='l'){
		if(buscarCadena(s,n,"\\land")){
			s[n]='*';
			s[n+1]=' ';s[n+2]=' ';s[n+3]=' ';s[n+4]=' ';
		}
		else{
			if(buscarCadena(s,n,"\\lor")){
				s[n]='+';
				s[n+1]=' ';s[n+2]=' ';s[n+3]=' ';
			}
		}
	}
	else{
		if(buscarCadena(s,n,"\\neg")){
			s[n]='~';
			s[n+1]=' ';s[n+2]=' ';s[n+3]=' ';
		}
		else{
			if(buscarCadena(s,n,"\\rightarrow")){
				s[n]='>';
				for(i=1;s[n+i]!='w';i++)
					s[n+i]=' ';
				s[n+i]=' ';
			}
		}
	}
}





/**
 * eliminarParentesisInnecesarios:
 * 
 * Funcion que elimina los parentesis de los 
 * extremos de una proposicion.
 * */
void eliminarParentesisInnecesarios(char *prop){
	unsigned int i=0;
	unsigned int largo=(unsigned int)strlen(prop);
	if(prop[0]=='(' && prop[largo-1]==')' && finalParentesis(prop,0)==largo-1)
	    prop[0]=prop[largo-1]=' ';
}

void eliminarNegacionesInnecesarias(char *prop){
    unsigned int i=0;
    unsigned int largo=(unsigned int)strlen(prop);
    for(;i<largo-1;i++)
        if(prop[i]=='~' && prop[i+1]=='~')
            prop[i]=prop[i+1]=' ';
}



/**
 * buscarCadena:
 * 
 * Funcion que busca una string sub en una cadena s
 * desde una posicion n.
 * */
unsigned int buscarCadena(char *s,int n,char *sub){
	unsigned int i=0;
	unsigned int largo=(unsigned int)strlen(sub);
	for(;i<largo;i++)
		if(s[n+i]!=sub[i])
			return 0;
	return 1;
}




/**
 * generarTabla:
 *
 * Funcion que genera una tabla en base a un
 * operador y dos tablas enviadas por parametro.
 *
 * */
void generarTabla(unsigned int *p,unsigned int *q,unsigned int **pq,char c){ //crea la tabla de una proposicion
	if(c=='>')
		*(pq)=im(p,q);
	else
		*(pq)=c=='*'?y(p,q):o(p,q);
}





unsigned int *negar(unsigned int *p){
	unsigned int i=0;
	unsigned int *np=(unsigned int*) calloc(cvalores,sizeof(unsigned int));
	for(;i<cvalores;i++)
		if(p[i])
			np[i]=0;
		else
			np[i]=1;
	return np;
}





unsigned int *im(unsigned int *p,unsigned int *q){
	unsigned int i=0,j=cvalores-1;
	unsigned int *pq=(unsigned int *) calloc(cvalores,sizeof(unsigned int));
	for(;i<j;i++,j--) {
        if (!(p[i] && !q[i]))
            pq[i] = 1;
        if (!(p[j] && !q[j]))
            pq[j] = 1;
	}
	return pq;
}





unsigned int *y(unsigned int *p,unsigned int *q){
    unsigned int i=0,j=cvalores-1;
    unsigned int *pq=(unsigned int *) calloc(cvalores,sizeof(unsigned int));
    for(;i<j;i++,j--) {
        if (p[i] && q[i])
            pq[i] = 1;
        if (p[j] && q[j])
            pq[j] = 1;
    }
	return pq;
}





unsigned int *o(unsigned int *p,unsigned int *q){
    unsigned int i=0,j=cvalores-1;
    unsigned int *pq=(unsigned int *) calloc(cvalores,sizeof(unsigned int));
    for(;i<j;i++,j--) {
        if (p[i] || q[i])
            pq[i] = 1;
        if (p[j] || q[j])
            pq[j] = 1;
    }
    return pq;
}




/**
 * finalParentesis:
 * 
 * Funcion que retorna la posici\'on correspondiente
 * al final del parentesis izquierdo que se encuentra
 * en la posicion inicio de la string prop.
 * 
 */
unsigned int finalParentesis(char *prop,unsigned int inicio){
	unsigned int i=inicio;
	unsigned int c=0;
	unsigned int largo=(unsigned int)strlen(prop);
	for(;i<largo;i++){
		if(prop[i]=='(')
			c++;
		if(prop[i]==')')
			c--;
		if(c==0)
			return i;				
	}
	return strlen(prop)-1;
}





/**----------------------------------------UTILIDADES-----------------------------------------------------**/
/**
 * charToString:
 * 
 * Funcion que da formato char* a un char.
 * 
 * */
char *charToString(char c){
	char s[2]={c,'\0'};
	return  strcpy((char*) calloc(2,sizeof(char)),s);
}




/**
 * subString:
 * 
 * Funcion que crea una substring de una string
 * desde una posicion de inicio hasta una final.
 * */
char *subString(char *prop,unsigned int inicio,unsigned int fin){
	unsigned int i,largo=fin-inicio;
	char *ss=(char*) calloc((largo+1),sizeof(char));
	for(i=inicio;i<=fin;i++)
		ss[i-inicio]=prop[i];
	ss[i-inicio]='\0';
	return ss;
}
/**----------------------------------------POSTFIX A INFIX-----------------------------------------------------**/
/**
 * postfixToInfix:
 * 
 * Funcion que transforma una expresion escrita en postfix a infix.
 * 
 * Le agrega todos los parentesis correspondientes, tal que la
 * expresion queda de la forma: ((p and q) or r) para la expresion
 * pq*r+.
 * */
char *postfixToInfix(char *s){
    unsigned int largo=strlen(s);
    unsigned int i;
    int k=-1;
    char **stack=(char **)calloc(largo,sizeof(char*));
    char *neg=(char*)calloc(largo,sizeof(char));
    for(i=0;i<largo;i++){
        if(isalpha(s[i]))
            stack[++k]=charToString(s[i]);
        else
            if(s[i]!='~')
                stack[--k]=operar(stack[k],stack[k+1],s[i]);
			else {
				neg[0] = '\0';
				strcat(neg, "~\0");
				strcat(neg, stack[k]);
				strcpy(stack[k], neg);
			}
    }
    return stack[0];
}
/**
 * operar:
 * 
 * Funcion que genera una expresion logica en base
 * a dos operando y un operador, dejandola de la forma:
 * 
 * (p op q)
 * 
 * Es utilizada en la funcion postfixToInfix para generar
 * la expresion infix.
 * */
char *operar(char *p,char *q,char op){
    char *final=(char*) calloc((strlen(p)+strlen(q)+10),sizeof(char));
    strcpy(final,"(");
    strcat(final,p);
	strcat(final,charToString(op));
    strcat(final,q);
    strcat(final,")");
	return final;
}

/**----------------------------------------INFIX A POSTFIX-----------------------------------------------------**/
/**
 * infixToPostfix:
 * 
 * Funcion que transforma una operacion infix a postfix.
 * 
 * Es utilizada para transformar la expresion principal y 
 * corregir la falta de parentesis. EJ: p and q or r debe
 * ser (p and q) or r.
 * 
 * */
char *infixToPostfix(char *s){
	unsigned int i;
	unsigned int largo=strlen(s);
	unsigned int swt=0;
	char actual;
	char *stack=crearStack(largo);
	char *stackOp=crearStack(largo);
	for(i=0;i<largo;i++){
		if(isalpha(s[i]))
			push(stack,s[i]);
		else{
			if(s[i]!=')'){
				if(stackVacio(stackOp) ||s[i]=='(')
					push(stackOp,s[i]);
				else{
					do{
						actual=pop(stackOp);
						if(prec(s[i])<=prec(actual)){
							if(actual!='(')
								push(stack,actual);
						}
						else{
							push(stackOp,actual);
							push(stackOp,s[i]);
							swt=1;
						}
					}while(!swt);
					swt=0;
				}
			}
			else{
				do{
					actual=pop(stackOp);
					if(actual!='(')
						push(stack,actual);
				}while(actual!='(');
			}
		}
	}
	while(!stackVacio(stackOp)){
		push(stack,pop(stackOp));
	}
	push(stack,'\0');
	return stack;
}
/**
 * prec:
 * 
 * Funcion que retorna la precedence o prioridad
 * de un operador logico. Si el operador no esta 
 * ingresado o es un caracter invalido, retorna -1.
 * */
int prec(char c){
    switch (c){
        case '~':
            return 4;
        case '*':
            return 3;
        case '+':
            return 2;
        case '>':
            return 1;
    	case '(':
    		return 0;
    }
    return -1;
}
/**
 * crearStack:
 * 
 * Funcion que crea un stack de largo l.
 * 
 * El formato del stack corresponde a una
 * cadena de caracteres con simbolos '-'
 * simulando un espacio vacio.
 * 
 * */
char *crearStack(unsigned int l){
	unsigned int i=0;
	char *stack=(char*)calloc(l,sizeof(char));
	stack[l]='\0';
	for(;i<l;i++)
		stack[i]='-';
	return stack;
}




unsigned int stackVacio(char *stack){
	return stack[0]=='-'?1:0;
}



/**
 * pop:
 * 
 * Funcion que elimina y retorna el 
 * ultimo caracter de un stack. Si el 
 * stack esta vacio, retorna '-'.
 * */
char pop(char *stack){
	unsigned int i=0;
	unsigned int largo=(unsigned int)strlen(stack);
	char ant;
	if(stack[0]=='-')
		return '-';
	for(;i<largo;i++)
		if(stack[i]=='-'){
			ant=stack[i-1];
			stack[i-1]='-';
			return ant;
		}
	return '-';
}




/**
 * push:
 * 
 * Funcion que inserta un caracter c
 * en la cima del stack. 
 * */
void push(char *stack,char c){
	unsigned int i=0;
	unsigned int largo=(unsigned int)strlen(stack);
	if(stack[0]=='\0')
		stack[0]=c;
	for(;i<largo;i++)
		if(stack[i]=='-'){
			stack[i]=c;
			return;
		}
}

/**----------------------------------------FIN PROGRAMA-----------------------------------------------------**/