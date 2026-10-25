# SRM University Coding Event Registration

A responsive SRM University coding-events website with a C++ registration server. Students can register for the CodeFest hackathon, a React workshop, a debugging contest, and an AI tech talk.

## Features

- Responsive campus events page with category filters and search.
- Registration form backed by a C++ HTTP server.
- Registrations are saved to a local CSV file. Do not commit registration data.

## Run locally on Windows

With MinGW-w64 installed, open PowerShell in this project directory:

```powershell
g++ -std=c++17 server.cpp -lws2_32 -o events-server.exe
.\events-server.exe
```

Open [http://localhost:8080](http://localhost:8080). Submit an event registration to create `registrations.csv` in the project directory. Stop the server with **Ctrl+C**.

## Deploy the website with GitHub Pages

The GitHub Actions workflow in `.github/workflows/pages.yml` builds and deploys the static website whenever a commit is pushed to `main`.

1. Push this project to the public repository's `main` branch.
2. In the repository, open **Settings → Pages** and set **Build and deployment → Source** to **GitHub Actions**.
3. After the workflow succeeds, the project website will be available at `https://vramanatham5.github.io/SRM-University-events-website/`.

GitHub Pages serves the website but cannot run its C++ registration server. To keep registrations working, deploy the C++ API on a Linux VPS:

1. Point a DNS **A record** for an API subdomain (for example, `events-api.example.com`) to your VPS public IPv4 address. Allow inbound TCP ports **80** and **443** in the VPS firewall/security group.
2. Install Docker Engine and the Docker Compose plugin on the VPS, then clone this repository.
3. In the project directory on the VPS, run `cp .env.example .env` and edit `.env`:
   - Set `DOMAIN` to the API subdomain (for example, `events-api.example.com`).
   - Set `PUBLIC_ORIGIN` to the exact website origin, `https://vramanatham5.github.io` (no repository path or trailing slash). If using a custom GitHub Pages domain, use that origin instead.
4. Start the API and HTTPS proxy with `docker compose up -d --build`. Caddy requests and renews the HTTPS certificate automatically.
5. In GitHub, open **Settings → Secrets and variables → Actions → Variables**, create a repository variable named `API_BASE_URL`, and set it to the API URL (for example, `https://events-api.example.com`).
6. Push a commit to `main` to redeploy Pages with the API URL. The registration form can then submit cross-origin requests to the API.

View VPS service logs with `docker compose logs -f`. Registration data persists in the Docker volume named `registrations`. The server also accepts `PORT`, `REGISTRATIONS_FILE`, and `PUBLIC_ORIGIN` environment variables. The `.gitignore` excludes `.env`, local registration data, compiled binaries, and editor settings. Student names, email addresses, and IDs are personal data; restrict VPS access and protect/backup registration data according to your university's policies.
