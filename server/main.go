package main

import (
	"fmt"
	"log"
	"net/http"
	"sync"

	ws "github.com/gorilla/websocket"
)

// a signal channel between two peers
type signalChannel struct {
	Peer [2]*ws.Conn
	Mux  sync.Mutex
}

var upgrader = ws.Upgrader{
	ReadBufferSize:  0,
	WriteBufferSize: 0,
	CheckOrigin: func(r *http.Request) bool {
		fmt.Print(r.URL.Host)
		return true // just in dev
	},
}

var (
	signalChannels    map[string]*signalChannel
	signalChannelsMux sync.Mutex
)

func (sc *signalChannel) addPeer(conn *ws.Conn) int {
	sc.Mux.Lock()
	defer sc.Mux.Unlock()

	if sc.Peer[0] == nil {
		sc.Peer[0] = conn
		return 0
	} else if sc.Peer[1] == nil {
		sc.Peer[1] = conn
		return 1
	}

	return -1 // crowded
}

func (sc *signalChannel) removePeerAndChannelIfEmpty(channelId string, peerId int) {
	sc.Mux.Lock()
	sc.Peer[peerId] = nil

	if sc.Peer[0] == nil && sc.Peer[1] == nil {
		sc.Mux.Unlock()
		signalChannelsMux.Lock()
		delete(signalChannels, channelId)
		signalChannelsMux.Unlock()
	} else {
		sc.Mux.Unlock()
	}
}

func (sc *signalChannel) sendMessage(peerId int, msg []byte) bool {
	sc.Mux.Lock()
	defer sc.Mux.Unlock()
	var err error = fmt.Errorf("not connected")

	if peerId == 0 {
		if sc.Peer[1] != nil {
			err = sc.Peer[1].WriteMessage(ws.TextMessage, msg)
			if err != nil {
				sc.Peer[1] = nil
			}
		}
	} else if sc.Peer[0] != nil {
		err = sc.Peer[0].WriteMessage(ws.TextMessage, msg)
		if err != nil {
			sc.Peer[0] = nil
		}
	}

	return err == nil
}

// id has to be 6 digits, A-Z, 0-9
func validChannelId(id string) bool {
	if len(id) != 6 {
		return false
	}

	for i := 0; i < len(id); i++ {
		if (id[i] < 'A' || id[i] > 'Z') && (id[i] < '0' || id[i] > '9') {
			return false
		}
	}

	return true
}

func writeHTTPError(w *http.ResponseWriter, code int) {
	http.Error(*w, http.StatusText(code), code)
}

func serveSignalChannel(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Access-Control-Allow-Origin", "*")
	channelId := r.URL.Path[4:]

	// ensure channel id is valid
	if !validChannelId(channelId) {
		writeHTTPError(&w, http.StatusBadRequest) // 400
		return
	}

	// upgrade request to a WebSocket connection
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		fmt.Printf("%v\n", err)
		writeHTTPError(&w, http.StatusInternalServerError) // 500
		return
	}
	defer conn.Close()

	fmt.Printf("Connection on ws://localhost:8080/ws/%s\n", channelId)
	defer fmt.Printf("Disconnection on ws://localhost:8080/ws/%s\n", channelId)

	// get signal channel, or create one if it doesn't exist
	signalChannelsMux.Lock()
	sc, exists := signalChannels[channelId]
	if !exists {
		sc = new(signalChannel)
		signalChannels[channelId] = sc
	}
	signalChannelsMux.Unlock()

	// only two peers per signal channel; get peer id or -1 if crowded
	peerId := sc.addPeer(conn)
	if peerId < 0 {
		writeHTTPError(&w, http.StatusServiceUnavailable) // 503
		return
	}
	defer sc.removePeerAndChannelIfEmpty(channelId, peerId)

	for {
		t, msg, err := conn.ReadMessage()
		if err != nil {
			fmt.Printf("%v\n", err)
			break
		}
		if t != ws.TextMessage {
			fmt.Printf("Bad message from %s\n", channelId)
			break
		}
		if !sc.sendMessage(peerId, msg) {
			conn.WriteMessage(ws.TextMessage, []byte("fail"))
		}
	}
}

func main() {
	signalChannels = make(map[string]*signalChannel)
	fmt.Printf("Listening on ws://localhost:8080/sc/...\n")
	http.HandleFunc("/sc/", serveSignalChannel)
	log.Fatal(http.ListenAndServe(":8080", nil))
}
