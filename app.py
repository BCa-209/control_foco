import time
from flask import Flask, render_template, request, jsonify

app = Flask(__name__)

# Estado global actualizado para 2 modos exactos
estado_sistema = {
    "modo": "AUTO",       # Puede ser "AUTO" o "MANUAL"
    "estado_manual": 0,   # 0 = Apagado, 1 = Encendido (Solo actúa si modo=="MANUAL")
    "relay": 0,           # La orden final que se envía al ESP32
    "ldr": 0,             # Última lectura del LDR
    "umbral": 300,        # Sensibilidad de oscuridad
    "conectado": False,
    "last_seen": 0        # Última vez que el ESP32 se comunicó
}

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/api/estado", methods=["GET"])
def obtener_estado():
    # Si han pasado más de 10 segundos sin señal del ESP32, marcar como desconectado
    if time.time() - estado_sistema.get("last_seen", 0) > 10:
        estado_sistema["conectado"] = False

    return jsonify(estado_sistema)

@app.route("/api/control", methods=["POST"])
def cambiar_control():
    data = request.json or {}
    
    if "modo" in data:
        estado_sistema["modo"] = data["modo"]
    
    if "estado_manual" in data:
        estado_sistema["estado_manual"] = data["estado_manual"]
        
    if "umbral" in data:
        estado_sistema["umbral"] = int(data["umbral"])

    # Si estamos en modo manual, la orden final del relé copia al estado manual
    if estado_sistema["modo"] == "MANUAL":
        estado_sistema["relay"] = estado_sistema["estado_manual"]

    return jsonify({"success": True, "estado": estado_sistema})

@app.route("/api/esp", methods=["GET"])
def endpoint_esp():
    ldr_valor = request.args.get("ldr", type=int)
    
    if ldr_valor is not None:
        estado_sistema["ldr"] = ldr_valor
        estado_sistema["conectado"] = True
        estado_sistema["last_seen"] = time.time()

        # Si estamos en modo automático, el servidor decide según el sensor
        if estado_sistema["modo"] == "AUTO":
            if ldr_valor > estado_sistema["umbral"]:
                estado_sistema["relay"] = 1 # Está oscuro -> Prender
            else:
                estado_sistema["relay"] = 0 # Hay luz -> Apagar

    # Se le envía la orden final al ESP32
    return jsonify({
        "relay": estado_sistema["relay"],
        "modo": estado_sistema["modo"]
    })

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)