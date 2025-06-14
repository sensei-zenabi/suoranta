#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri May  9 22:15:28 2025

Tip:
    !conda install seaborn --yes

Data Columns:
    0	Havaintoasema
    1	Vuosi
    2	Kuukausi
    3	Päivä
    4	Aika [Paikallinen aika]
    5	Kuukauden keskilämpötila [°C]
    6	Vuosi ja Kuukausi

@author: suoravi
"""

import pandas as pd;
import seaborn as sns;
import matplotlib.pyplot as plt

# %% Import and Process data

df = pd.DataFrame();

df = pd.read_csv('Jyväskylä lentoasema.csv');

df = df[['Vuosi','Kuukausi','Kuukauden keskilämpötila [°C]']]
df_v = df.groupby('Vuosi').mean().drop(labels='Kuukausi', axis=1).reset_index()

# %% Visualize data

# Plot with scatter + linear fit
plt.figure(figsize=(10,4))
sns.regplot(
    x='Vuosi', 
    y='Kuukauden keskilämpötila [°C]', 
    data=df_v,
    scatter_kws={'s':10, 'alpha':0.8},
    line_kws={'color':'red'}
)
plt.xlabel('Vuosi')
plt.ylabel('[°C]')
plt.title('Jyväskylän Lentoaseman Vuosittainen Keskilämpötila 1970-2024')
plt.grid(which='both',alpha=0.3)
plt.tight_layout()
plt.show()