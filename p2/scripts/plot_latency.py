import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

kernels = {
    "RBPi_no-RT": "Raspberry Pi (no-RT)",
    "RBPi_RT": "Raspberry Pi (RT)"
}

scenarios = {
    "S1": "Idle",
    "S2": "Hackbench",
    "S3": "Bonnie"
}

#Rango del eje X en microsegundos
x_min, x_max = 0, 60000 

#crear subplots para cada combinación de kernel y escenario
fig, axes = plt.subplots(len(kernels), len(scenarios), figsize=(15, 10),
                            sharex=True, sharey=True)
fig.suptitle("Distribucion de latencia entre kernel RT y no RT en raspberry pi")

for i, (kernel_key, kernel_label) in enumerate(kernels.items()):
    for j, (scenario_key, scenario_label) in enumerate(scenarios.items()):
        #Cambiar en funcion del directorio de trabajo
        filename = f"p2/CSVs/cyclictestURJC_data_{kernel_key}_{scenario_key}"

        try:
            data = pd.read_csv(filename, header=None, names=["CPU", "iteracion",
                                                                "latencia"])

            latencies = data["latencia"].values
            std_dev = np.std(latencies)
            
            axes[i, j].hist(latencies, bins=50, range=(x_min, x_max), alpha=0.7, 
                            label=f"{scenario_label} (std dev: {std_dev:.2f} µs)")
            

            axes[i, j].set_title(f"{kernel_label} - {scenario_label}")
            axes[i, j].set_xlabel("Latencia (µs)")
            axes[i, j].set_ylabel("Frecuencia")
            axes[i, j].legend()

        except FileNotFoundError:
            print(f"Archivo {filename} no encontrado.") 
            continue

# Ajustar para que los graficos no se solapen
plt.tight_layout(rect=[0, 0, 1, 0.95])

plt.savefig("latency_distribution.pdf", format="pdf")

plt.show()