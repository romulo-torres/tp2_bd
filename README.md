# 🗄️ Trabalho Prático 2 — Bancos de Dados I (2025/02)

**Universidade Federal do Amazonas – Instituto de Computação**  
**Professor:** Altigran Soares da Silva (<alti@icomp.ufam.edu.br>)  
**Disciplina:** Bancos de Dados I  
**Entrega:** 24/10/2025

---

## 👥 Equipe
- **Rômulo Fernandes Torres** — romulo.torres@icomp.ufam.edu.br  
- **Nycksandro Lima dos Santos** — nycksandro.santos@icomp.ufam.edu.br  
- **Luiz Henrique Barbosa Costa** — luiz.costa@icomp.ufam.edu.br

Planilha sobre as funções implementadas (quem fez, nome, o que faz):  
📄 [Google Sheets - Sobre as funções](https://docs.google.com/spreadsheets/d/12G-CmM8lXt1uVyELFymtKDpDNeO0uT-ksZgWiPU6krQ/edit?usp=sharing)

---

## 📘 Descrição do Trabalho

O projeto implementa um **sistema de armazenamento e consulta de registros em memória secundária**, utilizando:
- **Arquivo de dados organizado por hashing**
- **Índice primário** (B+Tree)
- **Índice secundário** (B+Tree)

A base de dados contém **artigos científicos** com os seguintes campos:

| Campo       | Tipo         | Descrição |
|--------------|--------------|------------|
| ID           | Inteiro      | Código identificador do artigo |
| Título       | Alfa (300)   | Título do artigo |
| Ano          | Inteiro      | Ano de publicação |
| Autores      | Alfa (150)   | Lista de autores |
| Citações     | Inteiro      | Quantidade de citações |
| Atualização  | Data/hora (20) | Última atualização |
| Snippet      | Alfa (100–1024) | Resumo textual do artigo (pode ser nulo) |

---

## 🧩 Estrutura dos Programas

| Programa | Função | Descrição |
|-----------|--------|------------|
| `upload <file>` | Carga inicial | Lê o CSV e cria os arquivos de dados e índices. |
| `findrec <ID>` | Busca direta | Lê diretamente no arquivo de dados pelo ID. |
| `seek1 <ID>` | Busca por índice primário | Usa a B+Tree primária (ID → posição no arquivo). |
| `seek2 <Título>` | Busca por índice secundário | Usa a B+Tree secundária (Título → posição no arquivo). |

Todos os programas medem:
- Quantidade de **blocos lidos**
- **Tempo de execução (ms)**
- **Caminhos dos arquivos usados**
- **Mensagens de log** configuráveis por `LOG_LEVEL` (`error`, `warn`, `info`, `debug`)

---

## 📂 Estrutura de Diretórios

## ඞ Como rodar ඞ
- Primeiro executar o limpa_csv.cpp pois ele irá gerar o artigo_novo.csv
- Daí, você executa o upload.cpp. Lembrando que a parte de hash está comentada.
