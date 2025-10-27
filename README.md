# Node Web Sockets Server for Arduino

 
 
## Test locally
- `npm install`
- `npm start`

You can test the sockets connection using a cli tool or [Insomnia](https://insomnia.rest/download)

```bash
npm install -g wscat
wscat -c ws://localhost:3000

# Then send messages:
{"type":"buttonPress"}
{"type":"brightness","value":200}

```

## Deploy on Render




