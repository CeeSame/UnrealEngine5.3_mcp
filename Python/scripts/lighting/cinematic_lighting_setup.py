#!/usr/bin/env python
"""
Cinematic Lighting Setup Script for Unreal Engine via MCP.

This script creates a professional cinematic three-point lighting setup:
1. Key Light - Main directional light (sun simulation)
2. Fill Light - Soft ambient fill
3. Back Light / Rim Light - Separation and depth
4. Sky Light - Environmental ambient

Usage: python cinematic_lighting_setup.py
"""

import sys
import os
import socket
import json
import logging
from typing import Dict, Any, Optional

# Set up logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger("CinematicLighting")

# Unreal Engine connection settings
UNREAL_HOST = "127.0.0.1"
UNREAL_PORT = 55557

def send_command(command: str, params: Dict[str, Any]) -> Optional[Dict[str, Any]]:
    """Send a command to Unreal Engine and get the response."""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect((UNREAL_HOST, UNREAL_PORT))

        try:
            command_obj = {
                "type": command,
                "params": params
            }
            command_json = json.dumps(command_obj)
            logger.info(f"Sending: {command_json}")
            sock.sendall(command_json.encode('utf-8'))

            # Receive response
            chunks = []
            while True:
                chunk = sock.recv(4096)
                if not chunk:
                    break
                chunks.append(chunk)
                try:
                    data = b''.join(chunks)
                    json.loads(data.decode('utf-8'))
                    break
                except json.JSONDecodeError:
                    continue

            data = b''.join(chunks)
            response = json.loads(data.decode('utf-8'))
            logger.info(f"Response: {response}")
            return response

        finally:
            sock.close()

    except Exception as e:
        logger.error(f"Error: {e}")
        return None

def spawn_light(name: str, light_type: str, location: list, rotation: list,
                intensity: float = 1.0, color: list = None,
                temperature: float = 6500.0, **kwargs) -> bool:
    """
    Spawn a light actor with specified properties.

    Args:
        name: Unique name for the light
        light_type: Type of light (DirectionalLight, PointLight, SpotLight, RectLight, SkyLight)
        location: [X, Y, Z] world position
        rotation: [Pitch, Yaw, Roll] in degrees
        intensity: Light intensity
        color: [R, G, B, A] color (0-1 range)
        temperature: Color temperature in Kelvin
        **kwargs: Additional properties
    """
    # Spawn the light actor
    params = {
        "name": name,
        "type": light_type,
        "location": location,
        "rotation": rotation
    }

    response = send_command("spawn_actor", params)
    if not response or response.get("status") not in ["success", None]:
        logger.error(f"Failed to spawn {name}: {response}")
        return False

    logger.info(f"Created {light_type}: {name}")

    # Set light properties
    if intensity != 1.0:
        send_command("set_actor_property", {
            "name": name,
            "property_name": "Intensity",
            "property_value": intensity
        })

    if color:
        # Convert to LinearColor format
        send_command("set_actor_property", {
            "name": name,
            "property_name": "LightColor",
            "property_value": color
        })

    if temperature != 6500.0 and light_type != "SkyLight":
        send_command("set_actor_property", {
            "name": name,
            "property_name": "Temperature",
            "property_value": temperature
        })

    # Set additional properties
    for prop_name, prop_value in kwargs.items():
        send_command("set_actor_property", {
            "name": name,
            "property_name": prop_name,
            "property_value": prop_value
        })

    return True

def create_cinematic_lighting():
    """
    Create a professional cinematic lighting setup.

    This follows the three-point lighting principle used in film:
    - Key Light: Primary illumination, creates shadows and highlights
    - Fill Light: Softens shadows, reduces contrast
    - Back/Rim Light: Separates subject from background, adds depth
    - Sky Light: Environmental ambient fill
    """

    logger.info("=" * 60)
    logger.info("Creating Cinematic Lighting Environment")
    logger.info("=" * 60)

    results = []

    # ============================================
    # 1. KEY LIGHT - Main Directional Light (Sun)
    # ============================================
    # 45-degree angle from front-right, simulating afternoon sun
    logger.info("\n[1/4] Creating Key Light (Directional)...")

    key_success = spawn_light(
        name="Cinematic_KeyLight",
        light_type="DirectionalLight",
        location=[500.0, 500.0, 800.0],
        rotation=[-45.0, 45.0, 0.0],  # 45° pitch down, 45° yaw
        intensity=10.0,
        color=[1.0, 0.95, 0.9, 1.0],  # Slightly warm white
        temperature=5500.0,  # Warm daylight
        LightSourceAngle=1.0,  # Soft shadows
        ShadowResolutionScale=2.0,  # High quality shadows
        CastShadows=True,
        DepthBias=0.5
    )
    results.append(("Key Light", key_success))

    # ============================================
    # 2. FILL LIGHT - Soft Point Light
    # ============================================
    # Positioned opposite to key light, lower intensity
    logger.info("\n[2/4] Creating Fill Light (Point)...")

    fill_success = spawn_light(
        name="Cinematic_FillLight",
        light_type="PointLight",
        location=[-400.0, 600.0, 300.0],
        rotation=[0.0, 0.0, 0.0],
        intensity=500.0,
        color=[0.9, 0.95, 1.0, 1.0],  # Slightly cool
        temperature=7500.0,  # Cool fill
        SourceRadius=200.0,  # Soft source
        SoftSourceRadius=150.0,
        AttenuationRadius=2000.0,
        CastShadows=False,  # Fill usually doesn't cast shadows
        UseInverseSquaredFalloff=True
    )
    results.append(("Fill Light", fill_success))

    # ============================================
    # 3. BACK LIGHT / RIM LIGHT - Spot Light
    # ============================================
    # Behind the subject, creating edge definition
    logger.info("\n[3/4] Creating Back/Rim Light (Spot)...")

    back_success = spawn_light(
        name="Cinematic_BackLight",
        light_type="SpotLight",
        location=[0.0, -600.0, 500.0],
        rotation=[-30.0, 180.0, 0.0],  # Angled down from behind
        intensity=3000.0,
        color=[1.0, 1.0, 1.0, 1.0],  # Neutral white
        temperature=6000.0,
        InnerConeAngle=30.0,
        OuterConeAngle=45.0,
        SourceRadius=50.0,
        SoftSourceRadius=100.0,
        AttenuationRadius=2000.0,
        CastShadows=False
    )
    results.append(("Back Light", back_success))

    # ============================================
    # 4. SKY LIGHT - Environmental Ambient
    # ============================================
    # Provides overall ambient illumination
    logger.info("\n[4/4] Creating Sky Light (Ambient)...")

    sky_success = spawn_light(
        name="Cinematic_SkyLight",
        light_type="SkyLight",
        location=[0.0, 0.0, 500.0],
        rotation=[0.0, 0.0, 0.0],
        intensity=1.0,
        SourceType=0,  # SLS_CapturedScene
        SkyDistanceThreshold=100000.0
    )
    results.append(("Sky Light", sky_success))

    # ============================================
    # 5. OPTIONAL: Accent Light - RectLight
    # ============================================
    # For cinematic look, add a rectangular soft light
    logger.info("\n[5/5] Creating Accent Light (Rect)...")

    accent_success = spawn_light(
        name="Cinematic_AccentLight",
        light_type="RectLight",
        location=[300.0, -200.0, 400.0],
        rotation=[-20.0, -30.0, 0.0],
        intensity=20.0,
        color=[1.0, 0.9, 0.8, 1.0],  # Warm accent
        temperature=4000.0,  # Warm tungsten
        SourceWidth=200.0,
        SourceHeight=100.0,
        AttenuationRadius=1500.0
    )
    results.append(("Accent Light", accent_success))

    # ============================================
    # Summary
    # ============================================
    logger.info("\n" + "=" * 60)
    logger.info("CINEMATIC LIGHTING SETUP COMPLETE")
    logger.info("=" * 60)

    success_count = sum(1 for _, success in results if success)
    total_count = len(results)

    for name, success in results:
        status = "✓" if success else "✗"
        logger.info(f"  {status} {name}")

    logger.info(f"\nTotal: {success_count}/{total_count} lights created successfully")

    if success_count == total_count:
        logger.info("\nAll lights created! Your cinematic lighting environment is ready.")
        logger.info("\nTips for fine-tuning:")
        logger.info("  - Adjust Key Light intensity for overall brightness")
        logger.info("  - Modify Fill Light to control shadow depth")
        logger.info("  - Tune Back Light for rim/separation effect")
        logger.info("  - Use Sky Light for ambient fill")

    return success_count == total_count

def main():
    """Main entry point."""
    try:
        success = create_cinematic_lighting()
        sys.exit(0 if success else 1)
    except Exception as e:
        logger.error(f"Fatal error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
