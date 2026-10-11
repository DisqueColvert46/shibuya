#ifndef CALCUL_H
#define CALCUL_H

void echangeLigne(int ligne, int colonne, float* matrice, int l1, int l2); // Echange de lignes
void chercheNonNul(int ligne, int colonne, float* matrice, float* aug, int index); // On cherche un élement non nul pour continuer le pivot
void addLigne(int ligne, int colonne, float* matrice, float* aug, int l1, int l2, float e); // l2 <- l2 + e*l1
void kLigne(int ligne, int colonne, float* matrice, int l, float e);
void modifIndex(int index, int ligne, int colonne, float* matrice, float* aug); // Descente jusqu'à transformer 'matrice' en matrice triangulaire supérieure
void modifIndexRetour(int index, int ligne, int colonne, float* matrice, float* aug); // Remontée pour générer l'inverse dans aug
void inverse(int ligne, int colonne, float* matrice, float* aug); // Calcul de l'inverse par la méthode de Gauss-Jordan
void aTransposeeFoisA(int ligne, int colonne, float* matriceA, float* sortie); // La matrice sortie doit être une matrice carrée de taille le nombre de colonnes de A
void produitMatriciel(int ligneA, int colonneA, float* matriceA, int ligneB, int colonneB, float* matriceB, float* sortie); // La matrice sortie doit être une matrice de dimension : nombre de ligne de A, nombre de colonne de B)
void transposee(int ligneA, int colonneA, float* matriceA, float* sortie); // Renvoie la transposée, le nombre de lignes de sortie sera colonneA, et le nombre de colonnes sera ligneA
void moindresCarres(int ligneA, int colonneA, float* matriceA, int ligneB, int colonneB, float* matriceB, float* sortie);

#endif