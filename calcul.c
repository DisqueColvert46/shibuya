#include <string.h>
#include <math.h>
#include "calcul.h"

void echangeLigne(int ligne, int colonne, float* matrice, int l1, int l2) { // Echange de lignes
	for (int j=0; j < colonne;j++) {
		float ech = matrice[l1*colonne + j];
		matrice[l1*colonne + j] = matrice[l2*colonne + j];
		matrice[l2*colonne + j] = ech;
	}
}

void chercheNonNul(int ligne, int colonne, float* matrice, float* aug, int index) { // On cherche un élement non nul pour continuer le pivot
	int v = index;
	while (matrice[v*colonne + index] == 0 && v < ligne) {
		v += 1;
	}
	echangeLigne(ligne, colonne, matrice, index, v);
	echangeLigne(ligne, colonne, aug, index, v);
}

void addLigne(int ligne, int colonne, float* matrice, float* aug, int l1, int l2, float e) { // l2 <- l2 + e*l1
	for (int j = 0; j < colonne; j++) {
		matrice[l2*colonne + j] += e*matrice[l1*colonne + j];
		aug[l2*colonne + j] += e*aug[l1*colonne + j];
	}
}

void kLigne(int ligne, int colonne, float* matrice, int l, float e) {
	for (int j = 0; j < colonne; j++) {
		matrice[l*colonne + j] *= e;
	}
}

void modifIndex(int index, int ligne, int colonne, float* matrice, float* aug) { // Descente jusqu'à transformer 'matrice' en matrice triangulaire supérieure
	chercheNonNul(ligne, colonne, matrice, aug, index);
	float e = (1 / matrice[index*colonne + index]);
	kLigne(ligne, colonne, matrice, index, e);
	kLigne(ligne, colonne, aug, index, e);
	for (int i = index+1;i < ligne; i++) {
		addLigne(ligne, colonne, matrice, aug, index, i, (0-1) * matrice[i*colonne + index]);
	}
}

void modifIndexRetour(int index, int ligne, int colonne, float* matrice, float* aug) { // Remontée pour générer l'inverse dans aug
	for (int i = 0;i < index; i++) {
		addLigne(ligne, colonne, matrice, aug, index, i, (0-1) * matrice[i*colonne + index]);
	}
}

void inverse(int ligne, int colonne, float* matrice, float* aug) { // Calcul de l'inverse par la méthode de Gauss-Jordan
	for (int i = 0; i < ligne; i++) {
		modifIndex(i, ligne, colonne, matrice, aug);
	}
	float def = 1.0f;
	for (int i = 0; i < ligne; i++) {
		def *= matrice[i*colonne+i];
	}
	if (fabs(def) > 1000.0f) { // Si la matrice n'est pas inversible, le vecteur solution n'est pas défini, on va donc le rendre nul plutôt que d'avoir une direction arbitraire qui gâchera les résultats
		for (int i = 0; i < ligne; i++) {
			for (int j = 0; j < colonne; j++) {
				aug[i*colonne+j] = 0;
			}
		}
	} else {
		for (int i = ligne-1; i >= 0; i--) {
			modifIndexRetour(i, ligne, colonne, matrice, aug);
		}
	}
}

void aTransposeeFoisA(int ligne, int colonne, float* matriceA, float* sortie) { // La matrice sortie doit être une matrice carrée de taille le nombre de colonnes de A
	for (int i = 0;i < colonne; i++) {
		for (int j = 0; j < colonne; j++) {
			sortie[i*colonne+j] = 0;
			for (int k = 0; k < ligne; k++) { // Formule du produit matriciel
				sortie[i*colonne+j] += matriceA[k*colonne+i]*matriceA[k*colonne+j];
			}
		}
	}
}

void produitMatriciel(int ligneA, int colonneA, float* matriceA, int ligneB, int colonneB, float* matriceB, float* sortie) { // La matrice sortie doit être une matrice de dimension : nombre de ligne de A, nombre de colonne de B)
	// On doit avoir colonneA == ligneB pour que le produit matriciel soit défini
	for (int i = 0;i < ligneA; i++) {
		for (int j = 0; j < colonneB; j++) {
			sortie[i*colonneB+j] = 0;
			for (int k = 0; k < colonneA; k++) { // Formule du produit matriciel
				sortie[i*colonneB+j] += matriceA[i*colonneA+k]*matriceB[k*colonneB+j];
			}
		}
	}
}

void transposee(int ligneA, int colonneA, float* matriceA, float* sortie) { // Renvoie la transposée, le nombre de lignes de sortie sera colonneA, et le nombre de colonnes sera ligneA
	for (int i = 0; i < ligneA; i++) {
		for (int j = 0; j < colonneA; j++) {
			sortie[j*ligneA + i] = matriceA[i*colonneA + j];
		}
	}
}

void moindresCarres(int ligneA, int colonneA, float* matriceA, int ligneB, int colonneB, float* matriceB, float* sortie) {
	/*Renvoie un vecteur V solution de l'équation matricielle A*V = B par la méthode des moindres carrées
	En notant ligneV et colonneV le nombre de lignes et colonnes de sortie (=V) on doit avoir :
	ligneA = ligneB, colonneA = ligneV, colonneV = colonneB */
	float aTa[colonneA * colonneA];
	aTransposeeFoisA(ligneA, colonneA, matriceA, aTa);
	
	float inverse_aTa[colonneA * colonneA];
	memset(inverse_aTa, 0, sizeof(inverse_aTa));
	for (int i = 0; i < colonneA; i++) {
		inverse_aTa[i*colonneA + i] = 1.0f;
	}
	inverse(colonneA, colonneA, aTa, inverse_aTa);
	
	float aT[ligneA * colonneA];
	transposee(ligneA, colonneA, matriceA, aT);
	
	float produit[colonneA * ligneA];
	produitMatriciel(colonneA, colonneA, inverse_aTa, colonneA, ligneA, aT, produit);
	produitMatriciel(colonneA, ligneA, produit, ligneB, colonneB, matriceB, sortie);
}