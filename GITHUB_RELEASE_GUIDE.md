# 📦 Guide pour créer une release GitHub

## 🎯 Étapes pour créer la release GiuChess 2.0

### 1. Préparation des fichiers
- ✅ Exécutable compilé : `GiuChess-2.0.exe`
- ✅ Notes de release : `RELEASE_NOTES.md`
- ✅ Documentation : `README.md`, `README_UCI.md`

### 2. Création de la release sur GitHub

#### Via l'interface web GitHub :

1. **Aller sur le repository** : https://github.com/giuliano-ippoliti/GiuChess

2. **Cliquer sur "Releases"** (dans la barre latérale droite)

3. **Cliquer sur "Create a new release"**

4. **Remplir les informations :**
   - **Tag version** : `v2.0.0`
   - **Release title** : `GiuChess 2.0.0 - Major Code Cleanup & UCI Support`
   - **Description** : Copier le contenu de `RELEASE_NOTES.md`

5. **Attacher les fichiers binaires :**
   - Glisser-déposer `GiuChess-2.0.exe`
   - Optionnel : Créer un ZIP avec l'exe + documentation

6. **Options recommandées :**
   - ☑️ **Set as the latest release**
   - ☐ **Set as a pre-release** (décocher)

7. **Publier** : Cliquer sur "Publish release"

### 3. Structure recommandée des assets

```
📦 Release Assets:
├── GiuChess-2.0.exe                 # Exécutable Windows
├── GiuChess-2.0-Windows.zip         # Package complet Windows
└── Source code (zip/tar.gz)         # Généré automatiquement par GitHub
```

### 4. Contenu du package Windows (optionnel)

Créer un ZIP `GiuChess-2.0-Windows.zip` contenant :
```
GiuChess-2.0-Windows/
├── GiuChess-2.0.exe
├── README.md
├── README_UCI.md
├── RELEASE_NOTES.md
└── scripts/
    └── start_GiuChess_UCI
```

### 5. Après la publication

La release sera disponible à :
- **URL directe** : https://github.com/giuliano-ippoliti/GiuChess/releases/tag/v2.0.0
- **Téléchargement direct** : https://github.com/giuliano-ippoliti/GiuChess/releases/download/v2.0.0/GiuChess-2.0.exe

### 6. Avantages de cette approche

✅ **Téléchargement direct** : Les utilisateurs peuvent télécharger l'exe sans compiler  
✅ **Statistiques** : GitHub track les téléchargements  
✅ **Versioning** : Historique clair des versions  
✅ **Professionnalisme** : Approche standard des projets open source  
✅ **Multi-plateforme** : Possibilité d'ajouter Linux/Mac plus tard  

### 7. Commandes Git pour tagger (optionnel)

```bash
# Créer un tag local
git tag -a v2.0.0 -m "GiuChess 2.0.0 - Major Code Cleanup & UCI Support"

# Pousser le tag
git push origin v2.0.0

# Puis créer la release sur GitHub avec ce tag
```

---

## 🚀 Résultat attendu

Une fois la release créée, les utilisateurs pourront :
1. Aller sur la page des releases
2. Télécharger directement `GiuChess-2.0.exe`
3. L'utiliser immédiatement avec Arena ou tout autre GUI UCI
4. Voir les notes de release détaillées
5. Accéder au code source si nécessaire

**C'est la méthode standard et professionnelle pour distribuer des logiciels sur GitHub !** 🏆
