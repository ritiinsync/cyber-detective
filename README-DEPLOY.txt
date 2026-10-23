CYBER DETECTIVE — DEPLOYMENT PACKAGE

Files:
- index.html: website frontend
- server.cpp: C++ backend, adapted for hosted environments (uses PORT and DATA_DIR environment variables and listens on all interfaces)
- Dockerfile: compiles and runs the C++ backend on Render or another Docker host
- cyber_detective.cpp: original OOP console-program source for your college demonstration

DEPLOY OVERVIEW (GitHub + Render)
1. Extract this folder.
2. Create a GitHub account if you don't already have one.
3. On GitHub, create a NEW PRIVATE repository named cyber-detective. Do not add a README, .gitignore, or license if you will upload these files directly.
4. Upload the files in this folder (including Dockerfile and .dockerignore) to the repository root. Keep index.html and server.cpp at the top level.
5. On Render, sign in and choose New > Web Service. Connect GitHub and select your repository.
6. Choose Docker as the runtime/environment. Select a region and an instance plan. Create the service.
7. In Render's service settings, configure environment variable DATA_DIR to /var/data.
8. For persistent local-file storage, add a persistent disk mounted at /var/data. Render persistent disks require a paid web-service plan. Without a persistent disk or external database, account/progress files on the default filesystem can be lost on restarts/redeploys/spin-downs.
9. Redeploy, wait for status Live, then open the public onrender.com URL. Share that URL with your group.

IMPORTANT SECURITY / DEMO LIMITATIONS
This is a classroom demo backend, not production-ready authentication. It uses a simple non-cryptographic password hash, has no rate limiting or secure session system, and its simple HTTP/JSON parsing is not designed for hostile public traffic. Do not use real/reused passwords or sensitive personal data. For a public project used by real users, replace authentication and file storage with a properly secured authentication system and database (for example, PostgreSQL), validate inputs robustly, and add rate limiting. Use only demo accounts/passwords.

RENDER FREE PLAN NOTE
Free web services can spin down after inactivity and their local filesystem is ephemeral. A persistent disk is not available on the free web-service plan. If persistent progress saving across restarts is a requirement, use a paid persistent disk or migrate progress/account data to a supported persistent database. Check Render's current pricing and plan limits before deploying.
