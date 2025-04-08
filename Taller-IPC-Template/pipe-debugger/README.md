# Mario Debug - Visualización de Pipes

Este proyecto utiliza Python para visualizar gráficamente conexiones entre procesos y pipes en un sistema Linux.

## 📦 Requisitos

- Python 3.6 o superior  
- `pip` (administrador de paquetes de Python)

## 🐍 Crear y activar un entorno virtual

Es recomendable usar un entorno virtual para evitar conflictos con otras dependencias del sistema.

### 1. Crear el entorno virtual

```bash
python3 -m venv venv
```

Esto creará una carpeta llamada `venv/` en tu directorio con el entorno virtual.

### 2. Activar el entorno virtual

- En **Linux/macOS**:

  ```bash
  source venv/bin/activate
  ```

- En **Windows (cmd)**:

  ```cmd
  venv\Scripts\activate
  ```

- En **Windows (PowerShell)**:

  ```powershell
  venv\Scripts\Activate.ps1
  ```

### 3. Instalar las dependencias

```bash
pip install -r requirements.txt
```

## ▶️ Ejecutar el script

Una vez dentro del entorno virtual, simplemente ejecutá tu script:

```bash
python mario-debug.py
```

## 🛑 Desactivar el entorno virtual

Cuando termines, podés salir del entorno con:

```bash
deactivate
```
