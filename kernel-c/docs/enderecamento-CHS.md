1. O que é CHS, na prática

Um disquete (ou HD antigo) é organizado fisicamente como uma pilha de discos giratórios, cada um com trilhas concêntricas, divididas em setores. A BIOS usa 3 números pra "apontar" pra um lugar exato nesse disco:

C = Cylinder (cilindro)  -> qual "anel" concêntrico (varia com o raio do disco)
H = Head     (cabeça)    -> qual face do disco (disquete tem 2 lados: cabeça 0 e cabeça 1)
S = Sector   (setor)     -> qual pedaço da trilha (numerado a partir de 1, não de 0!)
Visualizando um disco (vista de cima, um cilindro)
                    setor 1
                 ___________
              S18/           \S2
             /                 \
       S17  |                   |  S3
            |     CILINDRO 0    |
       S16  |      CABEÇA 0     |  S4
             \                 /
              \___S...________/
                 setor 10

Cada "fatia" é um setor de 512 bytes. Um disquete padrão de 1.44MB tem 18 setores por trilha.

Empilhando: cabeças e cilindros
Disquete (visto de lado):

  Cabeça 0  ══════════════════▶  (face de cima do disco)
  Cabeça 1  ══════════════════▶  (face de baixo do disco)

  Cilindro 0   Cilindro 1   Cilindro 2  ...
  (trilha mais (uma trilha  (mais uma
   externa)     pra dentro)  pra dentro)

Pra ler o disco inteiro em sequência, a ordem "natural" que a BIOS segue é:

Cilindro 0, Cabeça 0, Setores 1→18
Cilindro 0, Cabeça 1, Setores 1→18
Cilindro 1, Cabeça 0, Setores 1→18
Cilindro 1, Cabeça 1, Setores 1→18
...
2. A regra que causou o bug

Uma única chamada int 0x13 (função 0x02, "read sectors") não consegue ler além do fim da trilha atual.

Ou seja: se você está no Cilindro 0, Cabeça 0, começando no Setor 2, você só tem até o Setor 18 disponível nessa chamada — no máximo 17 setores (18 − 2 + 1). Pedir mais do que isso numa chamada só
faz a BIOS falhar (seta a carry flag, indicando erro) ou, dependendo da implementação do driver, travar num loop de erro.

O que aconteceu no nosso caso
Setor 1  -> boot sector (bootsect.asm), não faz parte da leitura
Setor 2  -> início do kernel.bin
...
Setor 18 -> último setor disponível NESSA trilha
                 SETORES DISPONÍVEIS NA TRILHA 0 (cabeça 0)
   ┌────┬────┬────┬────┬─── ... ───┬────┐
   │ S1 │ S2 │ S3 │ S4 │           │ S18│
   └────┴────┴────┴────┴─── ... ───┴────┘
     ↑    ↑______________________________↑
   boot         17 setores disponíveis
   sector       para o kernel (S2 a S18)
Pedimos dh = 16 setores → 16 ≤ 17 → cabe na trilha → funcionou
Kernel cresceu, pedimos dh = 64 → 64 >> 17 → estoura muito → travou (BIOS nem tenta, erro imediato)
Kernel precisava de 19 setores → 19 > 17 → estoura por pouco → travou também, mesmo sendo "só 2 setores a mais"

O erro não é sobre quantos bytes o kernel tem — é sobre onde, na geometria do disco, esses bytes caem.

3. A solução: ler por trilha, avançando cabeça/cilindro manualmente

Como a BIOS não atravessa trilhas sozinha, o bootloader precisa fazer isso na mão: ler o que cabe na trilha atual, depois pular pra próxima cabeça (ou cilindro) e continuar.

┌─────────────────────────┐     ┌─────────────────────────┐
│  Cilindro 0 / Cabeça 0   │     │  Cilindro 0 / Cabeça 1   │
│  Setores 2 → 18          │ ──▶ │  Setores 1 → 18          │ ──▶  ...
│  (17 setores lidos)      │     │  (mais 18 setores)       │
└─────────────────────────┘     └─────────────────────────┘
      1ª chamada BIOS                2ª chamada BIOS

Cada "salto" pra próxima trilha é uma nova chamada int 0x13, com CH/DH (cilindro/cabeça) incrementados e CL (setor) resetado pra 1.

Pseudocódigo da lógica robusta
setores_restantes = N
cilindro = 0
cabeca = 0
setor = 2   ; primeira leitura começa depois do boot sector

enquanto setores_restantes > 0:
    setores_nesta_trilha = min(setores_restantes, 18 - setor + 1)
    ler(cilindro, cabeca, setor, setores_nesta_trilha)
    setores_restantes -= setores_nesta_trilha

    setor = 1                  ; próxima trilha sempre começa no setor 1
    cabeca += 1
    se cabeca > 1:              ; disquete só tem 2 cabeças (0 e 1)
        cabeca = 0
        cilindro += 1
4. Por que isso importa daqui pra frente

Esse projeto vai crescer bastante — principalmente ao integrar o Doom, cujo arquivo WAD sozinho já passa de várias centenas de setores. A solução "manual" (chamar disk_load duas vezes, com números fixos) resolve agora, mas não escala. A versão com loop automático (seção 3) é a que vale implementar antes de embutir arquivos grandes no kernel, porque:

Não depende de recalcular limites toda vez que o binário cresce
Funciona pra qualquer tamanho de kernel/dados, sem tocar no bootloader de novo
É o padrão usado por praticamente todo bootloader real-world que ainda usa CHS (antes de migrar pra LBA/int 0x13 extendida)
5. Nota sobre LBA (alternativa moderna, fora de escopo por ora)

BIOS modernas também suportam LBA (Logical Block Addressing) via int 0x13 extendida (função 0x42), que trata o disco como uma lista simples de blocos numerados sequencialmente (0, 1, 2, 3...) — sem se preocupar com cilindro/cabeça/setor. É mais simples de programar e não tem o problema de "estourar trilha". Não é usado neste projeto por ora (o tutorial-base usa CHS clássico), mas é bom saber que existe como alternativa caso o projeto queira simplificar essa parte no futuro.
