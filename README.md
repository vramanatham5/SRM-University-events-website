# SRM Events Project

A responsive event-discovery and registration website for the SRM University student tech community. Browse coding events, filter by category, search by keyword, and register for a spot.

## Features

- Responsive event listings for hackathons, workshops, contests, and tech talks.
- Category filters, event search, and an accessible registration dialog.
- Registrations submitted to a Supabase `registrations` table.
- GitHub Actions workflow for deploying the static site to GitHub Pages.

## Run locally

The website is static. From the project directory, start a local web server:

```powershell
py -m http.server 8000
```

Then open [http://localhost:8000](http://localhost:8000). A local server is recommended instead of opening `index.html` directly so browser requests to Supabase work as expected.

## Configure registration

The browser submits registrations directly to Supabase. In `app.js`, set `SUPABASE_URL` to your project URL and `SUPABASE_KEY` to its **publishable (anon) key**. Never put a Supabase `service_role` key in client-side code.

Create a `registrations` table with columns matching the submitted fields:

| Column | Type | Notes |
| --- | --- | --- |
| `full_name` | `text` | Required |
| `email` | `text` | Required |
| `roll_number` | `text` | Required |
| `department` | `text` | Optional; defaults to an empty string |
| `event_name` | `text` | Required |

Enable row-level security and add an insert policy for the `anon` role. The site only needs permission to insert registrations; do not grant anonymous users read, update, or delete access. Protect the endpoint against spam before using it for a public production event.

## Deploy to GitHub Pages

The workflow in `.github/workflows/pages.yml` deploys the static website when changes are pushed to `main`.

1. Push the project to the repository's `main` branch.
2. In **Settings → Pages**, select **GitHub Actions** as the build and deployment source.
3. Check the **Actions** tab for the deployment result. The site will be available at `https://<owner>.github.io/<repository>/`.

## Registration data

Registration records contain student names, email addresses, and student IDs. Limit access to the Supabase project and handle collected data according to your university's privacy requirements.
