FROM node:20-alpine

# Install OpenSSL 3 libraries since openssl1.1-compat is removed in Alpine 3.22
RUN apk add --no-cache openssl

# Set working directory
WORKDIR /usr/src/app

# Copy project files (adjust as necessary)
COPY . .

# Default command (adjust as necessary)
CMD ["node", "index.js"]
