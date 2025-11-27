#!/usr/bin/env python3
"""
Script to parse NPC route images and extract polygons with specific color.
Extracts square polygons with color #A349A4 (purple) and ignores #ED1C24 (red).

Usage:
    python3 parse_npc_routes.py <city_name> [city_name2 ...]
    
Examples:
    python3 parse_npc_routes.py liberty_city
    python3 parse_npc_routes.py liberty_city san_andreas vice_city

Input:
    - Reads from: server/world/npcs/npcs_routes_<city>.png
    
Output:
    - Writes to: server/world/npcs/npcs_<city>.json
    - Format: JSON array of polygons, where each polygon is an array of [x,y] coordinates
    
Color codes:
    - #A349A4 (purple): NPC routes to extract
    - #ED1C24 (red): Polygons to ignore
"""

import json
import sys
from pathlib import Path
from typing import List, Tuple, Set
from PIL import Image


def hex_to_rgb(hex_color: str) -> Tuple[int, int, int]:
    """Convert hex color to RGB tuple."""
    hex_color = hex_color.lstrip('#')
    return tuple(int(hex_color[i:i+2], 16) for i in (0, 2, 4))


def find_polygons(image_path: str, target_color: str, ignore_color: str) -> List[List[List[int]]]:
    """
    Find square polygons in image with target color, ignoring those with ignore color.
    
    Args:
        image_path: Path to the PNG image
        target_color: Hex color to extract (e.g., '#A349A4')
        ignore_color: Hex color to ignore (e.g., '#ED1C24')
    
    Returns:
        List of polygons, where each polygon is a list of [x, y] coordinates
    """
    # Load image
    img = Image.open(image_path).convert('RGB')
    width, height = img.size
    
    target_rgb = hex_to_rgb(target_color)
    ignore_rgb = hex_to_rgb(ignore_color)
    
    # Track visited pixels
    visited = set()
    
    polygons = []
    
    # Scan the image for polygons
    for y in range(height):
        for x in range(width):
            pixel = img.getpixel((x, y))
            
            if pixel == target_rgb and (x, y) not in visited:
                # Found a new polygon, flood fill to find its bounds
                polygon_pixels = flood_fill(img, visited, x, y, target_rgb, width, height)
                
                if not polygon_pixels:
                    continue
                
                # Check if this polygon contains any ignore color pixels
                contains_ignore = False
                for px, py in polygon_pixels:
                    if img.getpixel((px, py)) == ignore_rgb:
                        contains_ignore = True
                        break
                
                if contains_ignore:
                    continue
                
                # Get bounding box
                xs = [p[0] for p in polygon_pixels]
                ys = [p[1] for p in polygon_pixels]
                
                min_x, max_x = min(xs), max(xs)
                min_y, max_y = min(ys), max(ys)
                
                # Create square polygon (top-left, top-right, bottom-right, bottom-left)
                polygon = [
                    [min_x, min_y],
                    [max_x + 1, min_y],
                    [max_x + 1, max_y + 1],
                    [min_x, max_y + 1]
                ]
                
                polygons.append(polygon)
    
    return polygons


def flood_fill(img: Image.Image, visited: Set[Tuple[int, int]], start_x: int, start_y: int, 
               target_color: Tuple[int, int, int], width: int, height: int) -> List[Tuple[int, int]]:
    """
    Flood fill algorithm to find all connected pixels of the same color.
    
    Args:
        img: PIL Image object
        visited: Set of visited (x, y) tuples
        start_x: Starting x coordinate
        start_y: Starting y coordinate
        target_color: RGB tuple of the color to match
        width: Image width
        height: Image height
    
    Returns:
        List of (x, y) tuples representing all pixels in this polygon
    """
    stack = [(start_x, start_y)]
    pixels = []
    
    while stack:
        x, y = stack.pop()
        
        if x < 0 or x >= width or y < 0 or y >= height:
            continue
        
        if (x, y) in visited:
            continue
        
        pixel = img.getpixel((x, y))
        if pixel != target_color:
            continue
        
        visited.add((x, y))
        pixels.append((x, y))
        
        # Check 4-connected neighbors
        stack.extend([
            (x + 1, y),
            (x - 1, y),
            (x, y + 1),
            (x, y - 1)
        ])
    
    return pixels


def main():
    if len(sys.argv) < 2:
        print("Usage: python parse_npc_routes.py <city_name> [city_name2 ...]")
        print("Example: python parse_npc_routes.py liberty_city san_andreas")
        sys.exit(1)
    
    cities = sys.argv[1:]
    
    target_color = '#A349A4'  # Purple - NPC routes to extract
    ignore_color = '#ED1C24'  # Red - Ignore these polygons
    
    script_dir = Path(__file__).parent
    npcs_dir = script_dir / 'server' / 'world' / 'npcs'
    
    for city in cities:
        image_path = npcs_dir / f'npcs_routes_{city}.png'
        output_path = npcs_dir / f'npcs_{city}.json'
        
        if not image_path.exists():
            print(f"Warning: Image not found: {image_path}")
            continue
        
        print(f"Processing {city}...")
        print(f"  Reading: {image_path}")
        
        polygons = find_polygons(str(image_path), target_color, ignore_color)
        
        print(f"  Found {len(polygons)} polygons with color {target_color}")
        
        # Write to JSON file
        with open(output_path, 'w') as f:
            json.dump(polygons, f, indent=4)
        
        print(f"  Wrote: {output_path}")
    
    print("\nDone!")


if __name__ == '__main__':
    main()
