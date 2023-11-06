To fork a GitLab repository and push it to GitHub, you can follow these general steps:

1. **Clone the GitLab Repository:**
   First, clone the repository from GitLab to your local machine. You will need Git installed for this. 

   ```bash
   git clone https://gitlab.com/kobily/sf2000_multicore
   ```

2. **Create a New GitHub Repository:**
   Go to GitHub and create a new repository where you want to host this project.

3. **Add GitHub as a Remote:**
   Inside the local repository you cloned from GitLab, add your GitHub repository as a remote.

   ```bash
   git remote add github https://github.com/madcock/sf2000_multicore
   ```

4. **Push to GitHub:**
   Push the code to your GitHub repository. If you want to push the entire history, use the `--mirror` option.

   ```bash
   git push --mirror github
   ```

   This will push all branches and tags to your GitHub repository.

5. **Update GitLab Repository:**
   To avoid confusion, you may want to remove the GitLab remote, so you don't accidentally push to GitLab instead of GitHub.

   ```bash
   git remote remove origin
   ```

6. **Syncing with Upstream:**
   If you want to keep the GitHub repository up to date with changes made in the original GitLab repository, you can add the GitLab repository as an upstream remote.

   ```bash
   git remote add upstream https://gitlab.com/kobily/sf2000_multicore
   ```

   Then, you can fetch changes from the upstream GitLab repository and merge or rebase them into your GitHub repository as needed.

   ```bash
   git fetch upstream
   git merge upstream/master
   ```

7. **Pull Requests and Collaboration:**
   Now, you can collaborate with others on GitHub by creating pull requests, managing issues, and so on.

Keep in mind that forking and pushing a repository in this way might lead to issues with contributors, as it can be confusing to have multiple copies of a repository across different platforms. Make sure that all contributors are aware of the new GitHub repository, and you should communicate clearly about which repository is the primary one for ongoing development.
