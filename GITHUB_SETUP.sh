#!/bin/bash
# Git Repository Setup and Push Instructions
# This script helps you push your local repository to GitHub

# =============================================================================
# STEP 1: Create Remote Repository on GitHub
# =============================================================================
# 1. Go to https://github.com/new
# 2. Fill in the following:
#    Repository name: Dijkstra
#    Description: High-performance Dijkstra's Algorithm with Sequential vs Parallel Implementation
#    Visibility: Public (or Private)
#    Do NOT initialize with README, .gitignore, or license (we already have these)
# 3. Click "Create repository"
# 4. You'll get a URL like: https://github.com/YOUR_USERNAME/Dijkstra.git

# =============================================================================
# STEP 2: Add Remote and Push (Run these commands)
# =============================================================================

# Replace YOUR_USERNAME with your actual GitHub username
GITHUB_USERNAME="YOUR_USERNAME"
REPO_URL="https://github.com/${GITHUB_USERNAME}/Dijkstra.git"

# Run these commands in your terminal:
echo "Setting up remote repository..."
git remote add origin "$REPO_URL"

echo "Pushing main branch..."
git branch -M main
git push -u origin main

# =============================================================================
# STEP 3: Verify Push
# =============================================================================
echo "Verifying push..."
git remote -v
git log --oneline

# =============================================================================
# QUICK REFERENCE COMMANDS
# =============================================================================

# View current repository status
# git status

# View commit history
# git log --oneline

# View remote information
# git remote -v

# Add files and commit
# git add <file>
# git commit -m "Your message"

# Push to GitHub
# git push

# Pull latest changes
# git pull

# =============================================================================
# NEXT STEPS AFTER PUSHING
# =============================================================================
# 1. Visit https://github.com/YOUR_USERNAME/Dijkstra
# 2. Add Topics: dijkstra, shortest-path, parallel-computing, tbb, c++
# 3. (Optional) Enable GitHub Pages for documentation
# 4. (Optional) Add GitHub Actions for CI/CD
# 5. (Optional) Create releases for tagged versions

# =============================================================================
# USEFUL GITHUB FEATURES TO ENABLE
# =============================================================================
# 1. Star the repository (show interest)
# 2. Enable Discussions (for Q&A)
# 3. Enable Issues (for bug tracking)
# 4. Create project boards (for task management)
# 5. Set up branch protection rules (for main branch)

# =============================================================================
# COMMON GIT WORKFLOWS
# =============================================================================

# Create and switch to a new branch
# git checkout -b feature/my-feature

# Make changes, then commit
# git add .
# git commit -m "Add my feature"

# Push feature branch
# git push -u origin feature/my-feature

# Create Pull Request on GitHub
# (GitHub will show a "Compare & pull request" button)

# After PR is merged, switch back to main and pull
# git checkout main
# git pull origin main

# Delete local branch
# git branch -d feature/my-feature

# =============================================================================
# TROUBLESHOOTING
# =============================================================================

# If you get "fatal: remote origin already exists"
# git remote remove origin
# git remote add origin <new-url>

# If authentication fails
# 1. Use SSH instead of HTTPS (recommended)
# 2. Or use GitHub Personal Access Token (PAT) instead of password

# To switch to SSH:
# git remote set-url origin git@github.com:YOUR_USERNAME/Dijkstra.git

# =============================================================================
# SSH SETUP (Optional but recommended)
# =============================================================================

# 1. Generate SSH key (if you don't have one)
# ssh-keygen -t ed25519 -C "your_email@example.com"

# 2. Add SSH key to GitHub
# - Go to https://github.com/settings/ssh/new
# - Paste your public key (~/.ssh/id_ed25519.pub)

# 3. Test SSH connection
# ssh -T git@github.com

# 4. Update remote to use SSH
# git remote set-url origin git@github.com:YOUR_USERNAME/Dijkstra.git

echo "Setup complete! Follow the steps above to push to GitHub."
