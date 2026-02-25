# VidCom GUI

Tento projekt je zamýšlen jako zápočtová práce pro PRC1

### Cíl projektu
```vidcom.py``` je skript, který umožňuje vzít libovolné video jako vstup, zvolit jeden z implementovaných video enkodérů a případně určit i cílovou velikost výstupu. Skript se pomocí knihovny ```ffmpeg``` pokusí co nejlépe zmenšit vstupní video s co nejmenší ztrátou kvality. 

#### Je možné zmenšovat ve dvou režimech: 

- na cílovou velikost
- automatické zmenšení se zachováním kvality

#### V současnosti mám implementované enkodéry: 

- VP9 `libvpx` (včetně 2-pass)
- H265 `libx265` (včetně 2-pass)
- rychlé kódování pomocí NVENC enkodérů
    * `h264_nvenc`
    * `hevc_nvenc`
- AV1 `libsvtav1`

Skript má velmi základní CLI rozhraní. Lze používat bez argumentů (zeptá se na všechny vstupní informace při spuštění) a s argumenty ve formě `python vidcom.py video režim velikost_v_MB`


Cíl tohoto projektu je napsat program podle [vzoru napsaného v Pythonu](./vzor/vidcom.py), který dokáže zmenšovat videa na danou velikost nebo připravit videa k zálohování. Program bude mít možnost ovládání pomocí jednoduchého CLI i GUI rozhraní. Zároveň bych chtěl přidat nové funkce, které nejsou přítomné v existujícím skriptu, jako je třeba volitelná nastavitelnost jednotlivých parametrů, snazší spouštění na více souborů najednou a náhled výsledku v aplikaci. 

### Inspirace
Inspiroval jsem se těmito projekty

- [Constrict](https://github.com/Wartybix/Constrict)
- [Handbrake](https://handbrake.fr/)

### Použité knihovny a nástroje
Projekt bude vyvíjen v jazyce **C++**. Zmenšování videí se bude provádět pomocí knihovny `ffmpeg`. GUI rozhraní bude vytvořeno pomocí `gtk4` a `libadwaita`. GTK4 rozhraní je psáno v jazyce C, proto pro svůj projekt použiji knihovnu `gtkmm-4.0`. Pro usnadnění sestavování budu využívat **Meson build system**. Aplikace je primárně určená pro běh na Linuxu. 

### Instalace
WIP

#### Linux

Nejdřív nainstalujte potřebné balíčky a závislosti. Také je nutné mít nainstalovaný `ffmpeg`. Například na Arch Linuxu: 

```sudo pacman -S gtk4 libadwaita jsoncpp meson ffmpeg```

Před sestavením je třeba spustit v kořenovém adresáři projektu příkaz: 


```meson setup build```

Pro sestavení přejděte do nového adresáře `build` a spusťte příkaz: 

```meson compile```

Spustitelný soubor se jmenuje `vidcom-gui` a nachází se v adresáři `build`. 

*Instalace do systému bude doplněna. *