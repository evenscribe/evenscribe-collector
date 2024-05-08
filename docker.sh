#/bin/sh
echo "Initiating Docker Sequence...."

path="/tmp/clickhouse"
data_path="$path/data"
logs_path="$path/logs"
migration_script="./init-db.sh"

if [ -d $path ]; then
    echo "Mount path exists."
    if [ "$1" = "--clean" ]; then
        echo "Cleaning mount path...."
        rm -rf $data_path/* $logs_path/*
    fi
else
    echo "Creating mount path...."
    mkdir $path
    mkdir $data_path $logs_path
fi

CONTAINER_NAME="clickhouse-db"

if [ -n "$(docker ps -f "name=$CONTAINER_NAME" -f "status=running" -q)" ]; then
    echo "The container '$CONTAINER_NAME' is already running.\n"
else
    if docker ps -a | grep -q "$CONTAINER_NAME"; then
        echo "Container $CONTAINER_NAME exists."
        echo "Running $CONTAINER_NAME ...."
        docker start $CONTAINER_NAME >/dev/null 2>&1
        echo "$CONTAINER_NAME started.\n"
    else
        echo "Container $CONTAINER_NAME does not exist."
        echo "Creating $CONTAINER_NAME ...."
        docker run -d \
            -v $(realpath $data_path):/var/lib/clickhouse/ \
            -v $(realpath $logs_path):/var/log/clickhouse-server/ \
            -v $(realpath $migration_script):/docker-entrypoint-initdb.d/init-db.sh \
            -p 19000:9000 \
            -p 8123:8123 \
            --name $CONTAINER_NAME \
            --ulimit nofile=262144:262144 \
            clickhouse/clickhouse-server >/dev/null 2>&1
        echo "$CONTAINER_NAME started.\n"
    fi
fi
