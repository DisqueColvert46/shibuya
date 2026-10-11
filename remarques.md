# Remarques

Il s'agit plus d'un petit projet ludique pour apprendre le C que d'un programme révolutionnaire, il y a donc beaucoup de commentaires à faire !

## Résolution du système

La résolution par élimination de Gauss-Jordan a une complexité $\mathrm{O}(n^3)$, on dispose de méthodes plus optimisées, en particulier pour une matrice de la forme $A^tA$. 

En consultant **calcul.c**, vous remarquerez qu'on calcule un déterminant intermédiaire dans le calcul de l'inverse pour savoir si celui va aboutir. En théorie, il suffit que le déterminant soit non nul pour que le calcul aboutisse. Ici, un déterminant trop faible, même si il est non nul, caractérise une matrice dont le calcul ne sera tout de même pas exploitable : là où une matrice singulière caractérise une fenêtre donc les pixels ne suivent pas du tout de direction privilégiée, une matrice inversible mais avec un déterminant trop faible renverra une direction à peine privilégiée. 
La question est donc pour l'utilisateur d'ajuster lui même à partir de quelle valeur du déterminant il considère que la direction privilégiée est vraiment pertinente pour ses calculs.

Dans la même philosophie, le programme calcule la norme du vecteur $V=(V_x,V_y)$, et si cette norme est trop faible on considérera de même que la donnée n'est pas assez pertinente.

## Bruit

La grande question dans tout programme de traitement de données, c'est le filtrage du bruit. Dans ce programme, on utilise une vidéo Youtube **compressée** du quartier de Shibuya au Japon, qui contient BEAUCOUP de sources lumineuses "parasites" (enseignes de magasins, lampadaires etc...). Il va s'en dire que ces bruits faussent une partie des résultats, et le rôle d'un **bon** programme est de réduire au minimul l'influence de ces bruits.
Voici une ébauche de solution :

Intuitivement, on peut décomposer l'image en deux parties :

 - Une partie constante : les sources lumineuses, les magasins, les panneaux, les batîments
 - Une partie variable : les voitures et piétons en mouvement

L'objectif du programme est de travailler **UNIQUEMENT** sur la deuxième partie (les objets en mouvement). On doit donc réussir à se débarasser du **fond**, la partie constante.

### Première approche :

La méthode de Lucas-Kanade n'utilise que l'intensité des pixels, on peut donc manipuler les images en noir et blanc, récupérer une image qui ne contient pas d'objets en mouvement, et la soustraire à toutes les images sur lesquelles on travaille.
Une image en noir et blanc est simplement une matrice dont les coefficient sont des entiers, il suffit alors de soustraire coefficient par coefficient.

### Deuxième approche :

Il est possible qu'on ne puisse pas avoir accès à la partie constante si ce qu'on étudie est toujours en mouvement (c'est d'ailleurs le cas du quartier de Shibuya). Il suffit alors d'effectuer un calcul de moyenne : On prend plusieurs images au fil du temps, ces images vont contenir des objets en mouvements à différents endroits à chaque fois, et donc on perd l'information de ces objets lors d'un calcul de moyenne, ce qui permet de ne récupérer que la partie constante.
Pour effectuer ce calcul, il suffit d'une simple addition coefficient par coefficient de toutes les matrices associés aux images, puis d'une division de la matrice obtenue par le nombre d'images impliquées (moyenne arithmétique).
La moyenne peut être pondérée, géométrique ou autre selon l'utilisation qu'on veut en faire et la nature de la vidéo utilisée par le programme.

### Troisième approche :

Il se peut que la partie constante évolue sur le long terme : Dans le cas du quartier de Shibuya, les sources lumineuses évoluent au fil de la journée (lumière homogène du Soleil pendant la journée, lumière des magasins et lampadaires le soir, présence ou absence des feux des voitures).
Il s'agit alors d'effectuer une moyenne que l'on réactualise au fil de la journée. Par exemple, on peut calculer la moyenne sur une demi-heure pour l'utiliser en tant que partie constante sur les images de la demi-heure qui suit.

Plus complexe, mais possiblement plus précis, on calcule la moyenne au fur et à mesure de la demi-heure, et on effectue quand même les calculs pour chaque image en considérant la moyenne calculée jusque là comme étant la partie constante :
En notant $P_i$ la matrice de l'image au temps $i$ (le temps $0$ étant la première image de la demi-heure, et $T$ la dernière), on effectue la méthode de Lucas-Kanade sur la matrice de pixels suivante :
$$\tilde{P_i} = P_i - \frac{1}{T}\sum_{j=0}^TP_j$$
(P.S : si une soustraction donne un résultat négatif, on pourra écrire sa valeur absolue)
Clairement, la moyenne sera de plus en plus précise, mais n'est donc pas précise au début, un programme adapté pourra pondérer la moyenne avec des coefficients adaptés selon l'utilisation et les données qu'on cherche à obtenir.

### Quatrième approche :

Un pixel de couleur est décrit par $(R,G,B)$ avec $R$, $G$ et $B$ les intensités (de 0 à 255 ou de 00 à FF selon le type utilisé) respectives du Rouge, Vert et Bleu.

On remarque alors qu'un pixel purement rouge $(255,0,0)$ et un pixel purement bleu $(0,0,255)$ seront les mêmes (si on considère que l'intensité lumineuse est la somme des trois intensités). Or, si on a ainsi un piéton en t-shirt bleu qui se balade dans une zone éclairée par le néon rouge d'un magasin, le programme ne le détectera pas.

Ainsi, avant d'utiliser la méthode de Lucas-Kanade sur les intensités lumineuses, on garde les images en couleurs, on effectue le calcul de moyenne avec les images en couleurs, on calcule la partie variable en couleur (par une des approches précédentes), et enfin on la convertit en nuance de gris.

Ainsi, un piéton bleu dans une zone éclairée en rouge sera vu comme un piéton lumineux dans une zone noir (puisqu'on a retiré l'éclairage rouge qui fait partie de la partie constante), et la méthode de Lucas-Kanade le détectera bien !
