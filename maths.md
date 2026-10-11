# Mathématiques
Dans ce document, on va survoler les mathématiques nécessaires à la compréhension du code et de la méthode de Lucas-Kanade
On suppose que le lecteur est familié avec le calcul matriciel et différentiel, les systèmes linéaires, ainsi que l'analyse de base pour une fonction de plusieurs variables réelles à valeurs réelles.

## Lucas-Kanade
L'interprétation physique est décrite dans le document lucas_kanade.md . Dans ce document, on utilise un développement de Taylor à l'ordre 1 d'une fonction de 3 variables réelles à valeurs réelles :

$$f(x+\delta x, y + \delta y, t + \delta t) = f(x,y,t) + \frac{\partial f}{\partial x}\delta x+\frac{\partial f}{\partial y}\delta y+\frac{\partial f}{\partial t}\delta t$$

L'hypothèse qui est faite (même si dans les faits, les calculs sont faits \'à la physicienne'), c'est que pour tout vecteur $(\delta x, \delta y, \delta t)$ utilisé (des variations de positions et d'un pas de temps d'un pixel d'image à un autre), ce vecteur appartient à une boule ouverte de centre $(0,0,0)$ et de rayon $\epsilon$ fixé où la fonction f (qui sera l'intensité) sera développable à l'ordre 1.

## Méthode des moindres carrés

La méthode de Lucas-Kanade va nous donner un système linéaire de la forme :

$$AV=B,\space \space (A,V,B) \in \text{M}_{N,2}(\mathbb{R}) \times \mathbb{R}^2 \times \mathbb{R}^N$$

On va avoir trois problèmes principaux qui empêcheront le système d'avoir une solution exacte:

 - La méthode de Lucas-Kanade est approximative, et soumise à des conditions pas toujours respectées
 - Les données qu'on utilise (tout particulièrement l'exemple proposé) contiennent beaucoup de bruit dû à la qualité et la compression de la vidéo
 - Le code n'est pas optimal, les choix de paramétrages sont naïfs (voir remarques.md) et l'approximation machine lors des calculs arithmétiques peut jouer beaucoup

On va donc utiliser la méthode des moindres carrées :
Si on ne peut pas avoir $AV=B$, on va s'en rapprocher le plus possible. On cherche alors le vecteur V qui minimise la quantité $\| AV-B \|^2$ toujours positive et nulle lorsque $V$ est solution du système (on utilise ici la norme issue du produit scalaire canonique de $\mathbb{R}^N$ noté $\langle \cdot ,\cdot \rangle$).
On dispose donc de l'application suivante, différentiable en V :

$$f: V \rightarrow \|AV-B\|^2$$

Calculons donc la différentielle, pour $V$ fixé et $H$ au voisinage de 0 :

$$\begin{align*}
f(V+H) &= \|A(V+H)-B\|^2 \\
&=\|AV-B\|^2 + 2\langle AV-B, AH\rangle + \|AH\|^2 \\
&=\|AV-B\|^2 + 2\langle A^tAV-A^tB, H\rangle + \mathrm{o}(\|H\|^2) \\
\end{align*}$$

On a ainsi le gradient de f en V : $\nabla f(V) =  A^tAV-A^tB$
On veut un vecteur V qui minimise f, il s'agit donc de résoudre $A^tAV-A^tB = 0$
On suppose alors que $A^tA$ est inversible : en effet, si cette matrice n'est pas inversible cela signifiera qu'il n'y a pas de direction privilégiée, c'est à dire qu'on calcule un mouvement qui se disperse de manière homogène autour de lui, donc le calcul ne donnera pas de résultat exploitable et dans le code, on traitera le cas suivant en ignorant celui-ci. Ainsi :
$$V = (A^tA)^{-1}A^tB$$
Pour calculer V, le code utilisera l'algorithme d'élimination de Gauss-Jordan (bien commenté dans calcul.c)
