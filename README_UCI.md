# GiuChess 2.0 - Support UCI

GiuChess a été mis à jour pour supporter le protocole UCI (Universal Chess Interface) en plus du protocole XBoard original.

## Nouveautés

- **Support UCI complet** : Compatible avec toutes les GUIs modernes
- **Double protocole** : XBoard (mode par défaut) et UCI
- **Interface moderne** : Prêt pour Arena, Cute Chess, Scid vs. PC, etc.

## Compilation

### Linux/Unix/WSL
```bash
make clean
make
```

### Windows avec MinGW/MSYS2
```bash
gcc -Wall -g -O4 -mno-bmi2 -lm -lpthread -o GiuChess-2.0.exe *.c
```

### Windows avec Visual Studio
```cmd
cl /O2 *.c /Fe:GiuChess-2.0.exe
```

## Utilisation

### Mode XBoard (par défaut)
```bash
./GiuChess-2.0
```

### Mode UCI
```bash
./GiuChess-2.0 uci
```

## Test du protocole UCI

### Test manuel
```bash
./GiuChess-2.0 uci
```

Puis tapez les commandes UCI :
```
uci
isready
position startpos
go depth 5
quit
```

### Avec une GUI moderne

1. **Arena** (gratuit) :
   - Télécharger depuis http://www.playwitharena.de/
   - Engines → Install New Engine → Sélectionner GiuChess-2.0
   - Cocher "UCI Protocol"

2. **Cute Chess** (gratuit) :
   - Tools → Settings → Engines → Add
   - Command: chemin vers GiuChess-2.0
   - Arguments: uci

3. **Scid vs. PC** (gratuit) :
   - Tools → Analysis Engines → New
   - Sélectionner GiuChess-2.0 avec argument "uci"

## Commandes UCI supportées

- `uci` - Identification du moteur
- `isready` - Test de disponibilité
- `ucinewgame` - Nouvelle partie
- `position startpos [moves ...]` - Position de départ + coups
- `position fen <fen> [moves ...]` - Position FEN + coups
- `go [depth <n>] [movetime <ms>] [wtime <ms>] [btime <ms>]` - Recherche
- `stop` - Arrêt de la recherche (TODO)
- `quit` - Quitter

## Options UCI

- `Hash` : Taille de la table de hachage (1-1024 MB) - TODO
- `Threads` : Nombre de threads (1-8) - TODO  
- `Depth` : Profondeur de recherche par défaut (1-10)

## Développement futur

- [ ] Implémentation complète du parsing FEN
- [ ] Table de transposition (Hash)
- [ ] Support multi-threading
- [ ] Commande `stop` pour arrêter la recherche
- [ ] Options configurables via `setoption`
- [ ] Recherche par temps avec gestion précise

## Compatibilité

Le moteur reste 100% compatible avec l'ancien protocole XBoard tout en ajoutant le support UCI moderne.

## Notes techniques

- Le code UCI est dans `uci.c` et `uci.h`
- La sélection du protocole se fait via l'argument de ligne de commande
- Les fonctions de recherche existantes sont réutilisées
- Conversion automatique entre les formats de coups
