const ws = new WebSocket("ws://127.0.0.1:31795");
const secret = Deno.env.get("IPC_SECRET");
const __dirname = import.meta.dirname;

function assert(value) {
  if (value) {
    console.log("passed");
  } else {
    console.trace(`${value} assertion failed`);
  }
}

function assertEquals(value1, value2) {
  if (value1 === value2) {
    console.log("passed");
  } else {
    console.trace(`${value1} != ${value2}: assertEquals failed`);
  }
}

function general_test() {
  const reqs = [
    {
      jsonrpc: "2.0",
      id: "1000",
    },
    {
      jsonrpc: "0",
      id: "1001",
    },
    {
      jsonrpc: "2.0",
      id: 1002,
    },
    {
      jsonrpc: "2.0",
      id: null,
    },
    {
      jsonrpc: "2.0",
      id: 1,
      method: "version",
      params: [`token:${secret}`],
    },
    {
      jsonrpc: "2.0",
      method: "version",
      params: [`token:${secret}`],
    },
  ];
  return reqs;
}

function general_test_assert(msg) {
  if (msg.id === "1000") {
    assertEquals(msg.error?.message, "Invalid method");
  } else if (msg.id === "1001") {
    assertEquals(msg.error?.message, "Server only support jsonrpc 2.0");
  } else if (msg.id === 1002) {
    assertEquals(msg.error?.message, "Invalid method");
  } else if (msg.id === null) {
    assertEquals(msg.error?.message, "id must be integer or string");
  } else if (msg.id === 1) {
    assert(msg.result?.version);
  }
}

function auth_test() {
  const reqs = [
    {
      jsonrpc: "2.0",
      method: "ping",
      id: 2000,
    },
    {
      jsonrpc: "2.0",
      id: 2001,
      method: "status",
      params: [`token:${secret}`],
    },
    {
      jsonrpc: "2.0",
      id: 2002,
      method: "status",
      params: ["token:wrongsecret"],
    },
    {
      jsonrpc: "2.0",
      id: 2003,
      method: "status",
      params: ["tokwrongformat"],
    },
  ];
  return reqs;
}

function auth_test_assert(msg) {
  switch (msg.id) {
    case 2000:
      assertEquals(msg.result, "pong");
      break;
    case 2001:
      assertEquals(msg.error?.message, "Missing argument");
      break;
    case 2002:
      assertEquals(msg.error?.message, "Wrong secret");
      break;
    case 2003:
      assertEquals(msg.error?.message, "Secret is not provided");
      break;
  }
}

function open_test() {
  const template = {
    jsonrpc: "2.0",
    method: "open",
  };
  const reqs = [
    {
      ...template,
      id: 3000,
      params: [`token:${secret}`, "https://google.com"],
    },
    {
      ...template,
      id: 3001,
      params: [`token:${secret}`, `${__dirname}/../src`],
    },
    {
      ...template,
      id: 3002,
      params: [`token:${secret}`, "randomwordsshouldfail"],
    },
    {
      ...template,
      id: 3003,
      params: [`token:${secret}`, `file://${__dirname}/../assets`],
    },
  ];
  return reqs;
}

function open_test_assert(msg) {
  switch (msg.id) {
    case 3000:
      assertEquals(msg.result, "OK");
      break;
    case 3001:
      assertEquals(msg.result, "OK");
      break;
    case 3002:
      assert(msg.error);
      break;
    case 3003:
      assertEquals(msg.result, "OK");
      break;
  }
}

function delete_test() {
  const template = {
    jsonrpc: "2.0",
    method: "delete",
  };
  const reqs = [
    {
      ...template,
      id: 4000,
      params: [`token:${secret}`, "/"],
    },
    {
      ...template,
      id: 4001,
      params: [`token:${secret}`, "/home/cudiph"],
    },
    {
      ...template,
      id: 4002,
      params: [
        `token:${secret}`,
        `${__dirname}/../build///linux/src//translations/`,
      ],
    },
    {
      ...template,
      id: 4003,
      params: [
        `token:${secret}`,
        `${__dirname}/../build/linux/src/aria2tray.p/main.cpp.o`,
      ],
    },
    {
      ...template,
      id: 4004,
      params: [`token:${secret}`, `/bin/sh`],
    },
    {
      ...template,
      id: 4005,
      params: [`token:${secret}`, `/root/.bash_history`],
    },
  ];
  return reqs;
}

function delete_test_assert(msg) {
  switch (msg.id) {
    case 4000:
      assertEquals(msg.error?.code, 403);
      break;
    case 4001:
      assertEquals(msg.error?.code, 403);
      break;
    case 4002:
      assertEquals(msg.result, "OK");
      break;
    case 4003:
      assertEquals(msg.result, "OK");
      break;
    case 4004:
      assertEquals(msg.error?.code, 422);
      break;
    case 4005:
      assertEquals(msg.error?.code, 404);
      break;
  }
}
function status_test() {
  const template = {
    jsonrpc: "2.0",
    method: "status",
  };
  const reqs = [
    {
      ...template,
      id: 5000,
      params: [`token:${secret}`, "/etc/passwd"],
    },
    {
      ...template,
      id: 5001,
      params: [`token:${secret}`, `${__dirname}/../src`],
    },
    {
      ...template,
      id: 5002,
      params: [`token:${secret}`, `${__dirname}/root/.bash_history`],
    },
    {
      ...template,
      id: 5003,
      params: [`token:${secret}`, `${__dirname}/sdfsdfsdfsdfsdslkdfkds`],
    },
  ];
  return reqs;
}

function status_test_assertion(msg) {
  switch (msg.id) {
    case 5000:
      assertEquals(msg.result?.type, "file");
      assertEquals(msg.result?.exist, true);
      break;
    case 5001:
      assertEquals(msg.result?.type, "folder");
      assertEquals(msg.result?.exist, true);
      break;
    case 5002:
      assertEquals(msg.result?.exist, false);
      break;
    case 5003:
      assertEquals(msg.result?.exist, false);
      break;
  }
}

function filePicker_test() {
  const template = {
    jsonrpc: "2.0",
    method: "filePicker",
  };
  const reqs = [
    {
      ...template,
      id: 6000,
      params: [`token:${secret}`, "folder"],
    },
    {
      ...template,
      id: 6001,
      params: [`token:${secret}`, "file"],
    },
    {
      ...template,
      id: 6002,
      params: [`token:${secret}`, "file", "PNG image (*.png)"],
    },
    {
      ...template,
      id: 6003,
      params: [`token:${secret}`, "file"], // cancel this to pass
    },
  ];
  return reqs;
}

function filePicker_test_assert(msg) {
  switch (msg.id) {
    case 6000:
      assert(msg.result?.selected);
      break;
    case 6001:
      assert(msg.result?.selected);
      break;
    case 6002:
      assert(msg.result?.selected);
      break;
    case 6003:
      assert(!msg.result?.selected);
      break;
    default:
      break;
  }
}

function vibe_check(response) {
  general_test_assert(response);
  auth_test_assert(response);
  open_test_assert(response);
  delete_test_assert(response);
  status_test_assertion(response);
  filePicker_test_assert(response);
}

ws.onopen = (event) => {
  const reqs = [
    ...general_test(),
    ...auth_test(),
    // ...open_test(),
    // ...delete_test(),
    // ...status_test(),
    // ...filePicker_test(),
  ];

  for (const req of reqs) {
    ws.send(JSON.stringify(req));
  }

  // batch testing
  // ws.send(JSON.stringify(reqs));
};

ws.onmessage = (ev) => {
  const data = JSON.parse(ev.data);
  if (data.slice) {
    console.log("=============batch=============");
    console.log(data);
    for (const res of data) {
      vibe_check(res);
    }
  } else {
    vibe_check(data);
    console.log(data);
  }
};
