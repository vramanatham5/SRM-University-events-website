FROM gcc:14-bookworm AS build

WORKDIR /src
COPY server.cpp .
RUN g++ -std=c++17 -O2 -Wall -Wextra -pedantic server.cpp -o /server

FROM debian:bookworm-slim

WORKDIR /app
RUN groupadd --system --gid 10001 app && \
    useradd --system --uid 10001 --gid app --no-create-home app && \
    mkdir -p /data && chown app:app /data
COPY --from=build /server /app/server
COPY index.html styles.css app.js ./
ENV PORT=8080
ENV REGISTRATIONS_FILE=/data/registrations.csv
EXPOSE 8080
USER app
CMD ["/app/server"]
