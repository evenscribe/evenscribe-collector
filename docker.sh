#/bin/sh
echo "Initiating Docker Sequence...."

C_PATH="/tmp/clickhouse"
DATA_PATH="$C_PATH/data"
LOGS_PATH="$C_PATH/logs"
MIGRATION_SCRIPT="./init-db.sh"
CONTAINER_NAME="clickhouse-db"
IMAGE_NAME="clickhouse/clickhouse-server"

if [ -d $path ]; then
    echo "Mount path exists."
    if [ "$1" = "--clean" ]; then
        echo "Cleaning mount path...."
        rm -rf $DATA_PATH/* $LOGS_PATH/*
    fi
else
    echo "Creating mount path...."
    mkdir $C_PATH
    mkdir $DATA_PATH $LOGS_PATH
fi

if docker info > /dev/null 2>&1; then
    echo "Docker is running..."
else
    echo "Starting Docker...."
    open --background -a Docker
    sleep 3
fi

if [ -n "$(docker ps -f "name=$CONTAINER_NAME" -f "status=running" -q)" ]; then
    echo "The container '$CONTAINER_NAME' is already running.\n"
else
        if docker ps -a | grep -q "$CONTAINER_NAME"; then
            echo "Container $CONTAINER_NAME exists."
            echo "Running $CONTAINER_NAME ...."
            docker start $CONTAINER_NAME >/dev/null 2>&1
            echo "$CONTAINER_NAME started.\n"
        else
            if docker image inspect "$IMAGE_NAME" &> /dev/null; then
                echo "The image $IMAGE_NAME exists."
            else
                docker pull $IMAGE_NAME
            fi
            echo "Container $CONTAINER_NAME does not exist."
            echo "Creating $CONTAINER_NAME ...."
            docker run -d \
                -v $(realpath $DATA_PATH):/var/lib/clickhouse/ \
                -v $(realpath $LOGS_PATH):/var/log/clickhouse-server/ \
                -v $(realpath $MIGRATION_SCRIPT):/docker-entrypoint-initdb.d/init-db.sh \
                -p 19000:9000 \
                -p 8123:8123 \
                --name $CONTAINER_NAME \
                --ulimit nofile=262144:262144 \
                clickhouse/clickhouse-server >/dev/null 2>&1
            echo "$CONTAINER_NAME started.\n"
        fi
fi
