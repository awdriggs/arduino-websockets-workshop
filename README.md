# Node Web Sockets Server for Arduino

 
 
## Test locally
- `npm install`
- `npm start`

Test the sockets connection using a tool
```bash
npm install -g wscat
wscat -c ws://localhost:3000

# Then send messages:
{"type":"buttonPress"}
{"type":"brightness","value":200}
```

or [Insomnia](https://insomnia.rest/download)


