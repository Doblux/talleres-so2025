import os
import re
import sys
import psutil
import subprocess
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import math
from collections import defaultdict
from subprocess import Popen,PIPE

def getPidOf(processName):
    for processData in psutil.process_iter(attrs=['pid', 'name']):
        if processData.info['name'] == processName:
            return processData.info['pid']
    return None

def getPIDsfromGPID(pid):
    try:
        pgid = os.getpgid(pid)
    except ProcessLookupError:
        return []

    processPIDs = []
    for proc in psutil.process_iter(attrs=["pid"]):
        try:
            if os.getpgid(proc.info["pid"]) == pgid:
                processPIDs.append(proc.info["pid"])
        except ProcessLookupError:
            continue

    return processPIDs

def clasifyPipe(pid, fd):
    firstCommand = f'ls -la /proc/{pid}/fd/{fd}'
    secondCommad = " | awk '{print $1}'"
    permissions = subprocess.run(firstCommand + secondCommad, shell=True, capture_output=True, text = True)
    permissionsString = permissions.stdout
    if 'r' in permissionsString:
        return 'r'
    if 'w' in permissionsString:
        return 'w'
    return None


def openPIDCenterPipes(pid, pipesDict):
    fdRoute = f"/proc/{pid}/fd"
    for fd in sorted(os.listdir(fdRoute)):  # Ordenado por file descriptor
        pipeTuple = getPipeInformation(fdRoute, fd)
        if pipeTuple is None: 
            continue
        if pid not in pipesDict:
            pipesDict[pid] = []
        pipesDict[pid].append((fd, pipeTuple[0], pipeTuple[1]))
    return pipesDict


def openPipesCenterPipes(pid, pipesDict):
    fdRoute = f"/proc/{pid}/fd"
    for fd in sorted(os.listdir(fdRoute)):
        pipeTuple = getPipeInformation(fdRoute, fd)
        if pipeTuple is None: 
            continue
        if pipeTuple not in pipesDict:
            pipesDict[pipeTuple] = []
        pipesDict[pipeTuple].append((fd,pid))
    return pipesDict

def getPipeInformation(fdRoute, fd):
    path = os.path.join(fdRoute, fd)
    try:
        link = os.readlink(path)  # Resolver el enlace simbólico
    except OSError:
        return None
    if "pipe" in link:
        match = re.match(r"pipe:\[(\d+)\]", link)
        pipePermission = clasifyPipe(pid, fd)
        if match:
            pipeTuple = (pipePermission, int(match.group(1)))
        return pipeTuple

def printPipesCenter(pipesDict):
    for (pipeType, pipeID), conexiones in sorted(pipesDict.items(), key=lambda x: (x[1], x[0])):
        print(f"PIPE {pipeID} ({pipeType})")
        print("+--------+--------+")
        print("|  PID   |  FD    |")
        print("+--------+--------+")
        for fd, pid in conexiones:
            print(f"| {str(pid).ljust(6)} | {str(fd).ljust(6)} |")
        print("+--------+--------+\n")

def printPIDCenter(pipesDict):
    for pid in sorted(pipesDict):
        print(f"PID: {pid}")
        permissions = defaultdict(list)
        for fd, pipeType, pipe in pipesDict[pid]:
            permissions[pipeType].append((pipe, fd))

        if 'r' in permissions:
            print("  Lecturas (r):")
            for pipe, fd in sorted(permissions['r'], key=lambda x: (x[0], int(x[1]))):
                print(f"    PIPE: {pipe} → FD: {fd}")
        if 'w' in permissions:
            print("  Escrituras (w):")
            for pipe, fd in sorted(permissions['w'], key=lambda x: (x[0], int(x[1]))):
                print(f"    PIPE: {pipe} → FD: {fd}")
        print("-----------------")

def transformPipeDictToConectionList(pipeDict):
    connections = []
    for (pipeType, pipeID), fdList in pipeDict.items():
        for fdStr, pid in fdList:
            fd = int(fdStr)
            connections.append((pipeType, pipeID, pid, fd))
    return connections

def graphConnections(connections, pidNames):
    # Agrupar conexiones por pipe
    pipes = defaultdict(list)
    for pipeType, pipeID, pid, fd in connections:
        pipes[pipeID].append((pipeType, pid, fd))

    fig, ax = plt.subplots(figsize=(20, 10))
    pos = {}
    edges = []

    # Configuración de layout
    piperPerRow = 3
    xGroupSpacing = 10
    xColSpacing = 3
    ySpacing = 1.5
    rowAndBase = 0
    thisRow = 0

    radius = 0.8  # radio de los círculos

    for idx, (pipeID, pipeConnections) in enumerate(pipes.items()):
        bloque = idx % piperPerRow
        if bloque == 0 and idx != 0:
            thisRow += 1

        readers = [c for c in pipeConnections if c[0] == 'r']
        writers = [c for c in pipeConnections if c[0] == 'w']
        
        n = max(len(readers), len(writers), 1) # Centrar de acuerdo a la mayor cantidad

        x_base = bloque * xGroupSpacing
        y_base = rowAndBase - thisRow * (n * ySpacing + 4)

        x_read = x_base
        x_pipe = x_base + xColSpacing
        x_write = x_base + 2 * xColSpacing

        y_centro = y_base
        y_read_start = y_centro + (len(readers) - 1) * ySpacing / 2
        y_write_start = y_centro + (len(writers) - 1) * ySpacing / 2

        pipeNode = f'PIPE {pipeID}'
        pos[pipeNode] = (x_pipe, y_centro)

        for i, (tipo, pid, fd) in enumerate(readers):
            proc_node = f'PID {pid}\nFD {fd}'
            y = y_read_start - i * ySpacing
            pos[proc_node] = (x_read, y)
            edges.append((proc_node, pipeNode, 'blue'))

        for i, (tipo, pid, fd) in enumerate(writers):
            proc_node = f'PID {pid}\nFD {fd}'
            y = y_write_start - i * ySpacing
            pos[proc_node] = (x_write, y)
            edges.append((pipeNode, proc_node, 'red'))

    # Dibujar nodos
    for node, (x, y) in pos.items():
        if node.startswith("PIPE"):
            ax.text(x, y, node, ha='center', va='center',
                    fontsize=10,
                    bbox=dict(boxstyle='round,pad=0.6', fc='lightgrey', ec='black', lw=1))  # padding aumentado
        else:
            circle = plt.Circle((x, y), radius, color='lightblue', ec='black', zorder=2)
            ax.add_patch(circle)
            ax.text(x, y, node, ha='center', va='center', fontsize=8, zorder=3)

    shortenAmount = 0.8  # distancia a recortar desde ambos extremos

    for src, dst, color in edges:
        x1, y1 = pos[src]
        x2, y2 = pos[dst]

        dx, dy = x2 - x1, y2 - y1
        dist = math.hypot(dx, dy)

        if dist == 0:
            continue  # evitar división por cero

        ux, uy = dx / dist, dy / dist
        x1_adj = x1 + ux * shortenAmount
        y1_adj = y1 + uy * shortenAmount
        x2_adj = x2 - ux * shortenAmount
        y2_adj = y2 - uy * shortenAmount

        ax.arrow(x1_adj, y1_adj, x2_adj - x1_adj, y2_adj - y1_adj,
                 head_width=0.2, head_length=0.3,
                 fc=color, ec=color, length_includes_head=True)

    # Título y leyenda
    #ax.set_title("Conexiones Pipes a procesos", fontsize=14)
    ax.axis('off')
    ax.set_aspect('equal')

    # Crear leyenda de nombres de procesos
    legend_entries = [f"PID {pid}: {name}" for pid, name in pidNames]
    legend_text = "\n".join(legend_entries)

    # Dibujar la leyenda en una caja de texto
    props = dict(boxstyle='round', facecolor='white', alpha=0.8)
    ax.text(0.01, 1, legend_text, transform=ax.transAxes,
        fontsize=9, verticalalignment='center', bbox=props)

    red_patch = mpatches.Patch(color='red', label='write')
    blue_patch = mpatches.Patch(color='blue', label='read')
    plt.legend(handles=[red_patch, blue_patch], loc='upper right', bbox_to_anchor=(1, 1))

    plt.tight_layout()
    plt.show()

def getProcessName(pid):
    with open(f"/proc/{pid}/comm", "r") as f:
        return f.read().strip()

def getProcessNamesLists(pids):
    listPIDNames = []
    for pid in pids:
        pidName = getProcessName(pid)
        listPIDNames.append((pid, pidName))
    return listPIDNames

def printProcessNames(pids):
    print(" ")
    print("PID y nombre de procesos: ")
    listPIDNames = getProcessNamesLists(pids)
    for pid, pidName in listPIDNames:
        print(f'    - {pid}: {pidName}')
    print(" ")

def printLogo():
    print('''
    
    ┌──────────────[MARIO DEBUG]─────────────────┐
  │  ███╗   ███╗ █████╗ ██████╗ ██╗ ██████╗    │
  │  ████╗ ████║██╔══██╗██╔══██╗██║██╔═══██╗   │
  │  ██╔████╔██║███████║██████╔╝██║██║   ██║   │
  │  ██║╚██╔╝██║██╔══██║██╔══██╗██║██║   ██║   │
  │  ██║ ╚═╝ ██║██║  ██║██║  ██║██║╚██████╔╝   │
  │  ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝ ╚═════╝    │
  │                                            │
  │  ██████╗ ███████╗██████╗ ██╗   ██╗ ██████╗ │
  │  ██╔══██╗██╔════╝██╔══██╗██║   ██║██╔════╝ │
  │  ██║  ██║█████╗  ███████╝██║   ██║██║  ███╗│
  │  ██║  ██║██╔══╝  ██╔══██╗██║   ██║██║   ██║│
  │  ██████╔╝███████╗██████║╚██████╔╝╚██████╔╝ │
  │  ╚═════╝ ╚══════╝╚═╝   ╚═╝ ╚═══╝  ╚═════╝  │
  └───────────────────────▼────────────────────┘
          ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
        ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
      ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
    ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
  ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓

    ''')

def returnHelpMessage():
    return '''
Herramienta para visualizar informacion de pipes de distintos procesos. Se le otorga al programa la informacion del nombre del proceso a analizar
Uso: programa [--text [--pid | --pipes]] | [--graphics] | [--help]

Opciones:
    --help              Muestra este mensaje.
    --graphics          Muestra la representación gráfica. (Valor por defecto si no se pasa ninguna opción).
    --text              Muestra la representación de los pipes abiertos en texto.
        --pid <pid>       (Opcional) Muestra solo la información de los pipes abiertos por proceso.
        --pipes           (Opcional) Muestra la información de todos los pipes y que procesos lo tienen abierto. (Valor por defecto si se omite --pid).
    '''

if __name__ == "__main__":
    modeText = 'graphics'
    mode = 'pid'
    print("Recuerde que puede usar un modo texto con --text. Use --help para ver mas detalles")
    print("")
    if len(sys.argv) == 2:
        modeText = sys.argv[1]
        if modeText == '--help':
            print(returnHelpMessage())
            sys.exit(0)
        if modeText == '--logo':
            printLogo()
    if len(sys.argv) > 2:
        modeText = sys.argv[1]
        mode = sys.argv[2]    

    filename = input("Ingresa el nombre del proceso: ")
    pid = getPidOf(filename)
    if pid is None:
        print(f"No se encontro un proceso asociado a ese nombre")
        sys.exit()
    pids = getPIDsfromGPID(pid)

    pipesDict = {}

    if modeText == '--text':
        modeText = 'text'
        if mode == '--pid':
            mode = 'pid'
        else:
            mode = 'pipe'
    
    if modeText == 'text':
        printProcessNames(pids)
        if mode == "pipe":
            for pid in pids:
                pipesDict = openPipesCenterPipes(pid, pipesDict)
            printPipesCenter(pipesDict)
        if mode == "pid":
            for pid in pids:
                pipesDict = openPIDCenterPipes(pid, pipesDict)
            printPIDCenter(pipesDict)
    else:
        for pid in pids:
            pipesDict = openPipesCenterPipes(pid, pipesDict)
        connections = transformPipeDictToConectionList(pipesDict)
        graphConnections(connections, getProcessNamesLists(pids))





        
    