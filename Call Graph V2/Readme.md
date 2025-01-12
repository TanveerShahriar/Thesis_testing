Install docker compose V2

```bash
sudo apt-get install -y docker-compose-plugin
```

Go to the directory where docker-compose.yml is
```bash
docker compose up
docker exec -it my_clang_container /bin/bash
```

Stop services without removing containers
```bash
docker compose stop
```

Stop and remove containers, networks, and other resources
```bash
docker compose down
```
