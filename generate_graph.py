#!/usr/bin/env python3
"""
Graph Generator for Dijkstra's Algorithm Testing
Generates graphs in DIMACS format (.gr files)
"""

import random
import argparse
import sys


def generate_small_graph(filename, nodes=4, edges_per_node=1.25):
    """Generate a small test graph"""
    with open(filename, 'w') as f:
        num_edges = int(nodes * edges_per_node)
        f.write(f'c Small test graph\n')
        f.write(f'p sp {nodes} {num_edges}\n')
        
        edge_count = 0
        for i in range(1, nodes + 1):
            for j in range(int(edges_per_node)):
                target = (i % nodes) + 1
                weight = random.randint(1, 20)
                f.write(f'a {i} {target} {weight}\n')
                edge_count += 1
                if edge_count >= num_edges:
                    break
            if edge_count >= num_edges:
                break
    
    print(f"✓ Generated small graph: {filename}")
    print(f"  Nodes: {nodes}, Edges: {edge_count}")


def generate_medium_graph(filename, nodes=100, edges_per_node=5):
    """Generate a medium-sized graph"""
    with open(filename, 'w') as f:
        num_edges = nodes * edges_per_node
        f.write(f'c Medium test graph\n')
        f.write(f'p sp {nodes} {num_edges}\n')
        
        edge_count = 0
        for i in range(1, nodes + 1):
            for j in range(edges_per_node):
                target = random.randint(1, nodes)
                weight = random.randint(1, 50)
                f.write(f'a {i} {target} {weight}\n')
                edge_count += 1
    
    print(f"✓ Generated medium graph: {filename}")
    print(f"  Nodes: {nodes}, Edges: {edge_count}")


def generate_large_graph(filename, nodes=1000, edges_per_node=5):
    """Generate a large graph"""
    with open(filename, 'w') as f:
        num_edges = nodes * edges_per_node
        f.write(f'c Large test graph\n')
        f.write(f'p sp {nodes} {num_edges}\n')
        
        edge_count = 0
        for i in range(1, nodes + 1):
            for j in range(edges_per_node):
                target = random.randint(1, nodes)
                weight = random.randint(1, 100)
                f.write(f'a {i} {target} {weight}\n')
                edge_count += 1
    
    print(f"✓ Generated large graph: {filename}")
    print(f"  Nodes: {nodes}, Edges: {edge_count}")


def generate_massive_graph(filename, nodes=5000, edges_per_node=10):
    """Generate a massive graph"""
    with open(filename, 'w') as f:
        num_edges = nodes * edges_per_node
        f.write(f'c Massive test graph\n')
        f.write(f'p sp {nodes} {num_edges}\n')
        
        edge_count = 0
        for i in range(1, nodes + 1):
            for j in range(edges_per_node):
                target = random.randint(1, nodes)
                weight = random.randint(1, 100)
                f.write(f'a {i} {target} {weight}\n')
                edge_count += 1
    
    print(f"✓ Generated massive graph: {filename}")
    print(f"  Nodes: {nodes}, Edges: {edge_count}")


def generate_custom_graph(filename, nodes, edges_per_node, max_weight=100):
    """Generate a custom graph with specified parameters"""
    with open(filename, 'w') as f:
        num_edges = nodes * edges_per_node
        f.write(f'c Custom test graph\n')
        f.write(f'p sp {nodes} {num_edges}\n')
        
        edge_count = 0
        for i in range(1, nodes + 1):
            for j in range(edges_per_node):
                target = random.randint(1, nodes)
                weight = random.randint(1, max_weight)
                f.write(f'a {i} {target} {weight}\n')
                edge_count += 1
    
    print(f"✓ Generated custom graph: {filename}")
    print(f"  Nodes: {nodes}, Edges: {edge_count}, Max Weight: {max_weight}")


def main():
    parser = argparse.ArgumentParser(
        description='Generate DIMACS format graphs for Dijkstra testing',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 generate_graph.py --small
  python3 generate_graph.py --medium
  python3 generate_graph.py --large
  python3 generate_graph.py --massive
  python3 generate_graph.py --custom --nodes 10000 --edges-per-node 20
  python3 generate_graph.py --all
        """
    )
    
    parser.add_argument('--small', action='store_true', help='Generate small test graph (4 nodes)')
    parser.add_argument('--medium', action='store_true', help='Generate medium graph (100 nodes)')
    parser.add_argument('--large', action='store_true', help='Generate large graph (1000 nodes)')
    parser.add_argument('--massive', action='store_true', help='Generate massive graph (5000 nodes)')
    parser.add_argument('--custom', action='store_true', help='Generate custom graph')
    parser.add_argument('--all', action='store_true', help='Generate all test graphs')
    
    parser.add_argument('--nodes', type=int, default=1000, help='Number of nodes (for custom)')
    parser.add_argument('--edges-per-node', type=float, default=5, help='Edges per node (for custom)')
    parser.add_argument('--max-weight', type=int, default=100, help='Maximum edge weight')
    parser.add_argument('--output-dir', default='.', help='Output directory for graphs')
    
    args = parser.parse_args()
    
    if not any([args.small, args.medium, args.large, args.massive, args.custom, args.all]):
        parser.print_help()
        return
    
    print("=" * 70)
    print("DIJKSTRA GRAPH GENERATOR")
    print("=" * 70)
    
    if args.small or args.all:
        generate_small_graph(f'{args.output_dir}/test.gr', nodes=4)
    
    if args.medium or args.all:
        generate_medium_graph(f'{args.output_dir}/large_graph.gr', nodes=100)
    
    if args.large or args.all:
        generate_large_graph(f'{args.output_dir}/huge_graph.gr', nodes=1000)
    
    if args.massive or args.all:
        generate_massive_graph(f'{args.output_dir}/massive_graph.gr', nodes=5000)
    
    if args.custom:
        filename = f'{args.output_dir}/custom_graph.gr'
        generate_custom_graph(
            filename,
            nodes=args.nodes,
            edges_per_node=int(args.edges_per_node),
            max_weight=args.max_weight
        )
    
    print("=" * 70)


if __name__ == '__main__':
    main()
