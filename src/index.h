#pragma once

constexpr char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta
      name="viewport"
      content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no"
    />
    <title>Square Rats Totem</title>
    <style>
      * {
        box-sizing: border-box;
        margin: 0;
        padding: 0;
      }
      body {
        font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto,
          sans-serif;
        background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        min-height: 100vh;
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: start;
        justify-items: center;
        padding: 1rem;
        gap: 1rem;
      }
      .container {
        background: rgba(0, 0, 0, 1);
        border-radius: 20px;
        padding: 1.5rem;
        max-width: 400px;
        width: 100%;
        margin-top: 3rem;
        box-shadow: 0 20px 40px rgba(0, 0, 0, 0.1);
      }
      h1 {
        text-align: center;
        color: white;
        margin-bottom: 30px;
        font-size: 28px;
      }
      .rat-emoji {
        font-size: 40px;
        display: block;
        text-align: center;
        margin-bottom: 0.75rem;
      }
      .control-group {
        margin-bottom: 25px;
      }
      .control-group h3 {
        color: #a1a1a1;
        margin-bottom: 15px;
        font-size: 18px;
      }
      button {
        width: 100%;
        padding: 15px;
        border: none;
        border-radius: 10px;
        font-size: 16px;
        font-weight: 600;
        cursor: pointer;
        transition: all 0.3s ease;
        margin-bottom: 10px;
      }
      .btn-primary {
        background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
        color: white;
      }
      .btn-primary:active {
        transform: scale(0.98);
      }
      .btn-secondary {
        color: #393939;
        background: white;
      }
      .btn-secondary:active {
        background: #e0e0e0;
      }
      .brightness-control {
        margin: 20px 0;
      }
      input[type='range'] {
        width: 100%;
      }
      .brightness-value {
        text-align: center;
        font-size: 24px;
        font-weight: bold;
        color: #667eea;
        margin-top: 10px;
      }
      .status {
        position: fixed;
        top: 0;
        z-index: 1000;
        padding: 10px;
        width: 90%;
        border-radius: 8px;
        text-align: center;
        margin: 0.75rem;
        font-weight: 600;
        transition: all 0.3s ease;
      }
      .status.success {
        color: #d4edda;
        background: #155724;
      }
      .status.error {
        color: #f8d7da;
        background: #721c24;
      }
      .upload-area {
        border: 2px dashed #ccc;
        border-radius: 10px;
        padding: 20px;
        text-align: center;
        margin: 10px 0;
      }
      input[type='file'] {
        display: none;
      }
      .upload-label {
        display: inline-block;
        padding: 10px 20px;
        background: #f0f0f0;
        border-radius: 8px;
        cursor: pointer;
      }
      .progress-bar {
        width: 100%;
        height: 20px;
        background-color: #f0f0f0;
        border-radius: 10px;
        overflow: hidden;
        margin: 10px 0;
        display: none;
      }
      .progress-fill {
        height: 100%;
        background: linear-gradient(90deg, #667eea 0%, #764ba2 100%);
        transition: width 0.3s;
        width: 0%;
      }
      .btn-pattern {
        background: linear-gradient(135deg, #764ba2 0%, #667eea 100%);
        color: white;
        font-size: 14px;
        padding: 12px;
        white-space: nowrap;
        overflow: hidden;
        text-overflow: ellipsis;
      }
      .btn-pattern:active {
        transform: scale(0.98);
        background: linear-gradient(135deg, #5a3a7a 0%, #4a6bb8 100%);
      }
      .patterns-grid {
        display: grid;
        grid-template-columns: 1fr 1fr;
        gap: 6px;
        margin-top: 12px;
      }
      .loading-text {
        color: #a1a1a1;
        font-style: italic;
        text-align: center;
        padding: 20px;
      }
    </style>
  </head>
  <body>
    <div id="status" class="status" style="display: none"></div>
    <div class="container">
      <span class="rat-emoji">🐀</span>
      <h1>Square Rats Totem</h1>

      <div class="control-group">
        <h3>Quick Controls</h3>
        <button class="btn-primary" onclick="sendCommand('ping')">
          Test Connection
        </button>
        <button class="btn-secondary" onclick="sendCommand('music')">
          Music Mode
        </button>
      </div>

      <div class="control-group">
        <h3>Brightness</h3>
        <div class="brightness-control">
          <input
            type="range"
            id="brightness"
            min="0"
            max="255"
            value="200"
            oninput="updateBrightness(this.value)"
          />
          <div class="brightness-value" id="brightnessValue">200</div>
        </div>
      </div>

      <div class="control-group">
        <h3>GIF Upload</h3>
        <div class="upload-area">
          <label for="gifFile" class="upload-label">Choose GIF File</label>
          <input
            type="file"
            id="gifFile"
            accept=".gif"
            onchange="fileSelected()"
          />
          <p id="fileName" style="margin-top: 10px; color: #dadada"></p>
        </div>
        <button class="btn-primary" onclick="uploadGif()">Upload GIF</button>
        <div class="progress-bar" id="progressBar">
          <div class="progress-fill" id="progressFill"></div>
        </div>
      </div>

      <div class="control-group">
        <h3>Pattern Selection</h3>
        <div id="patternsContainer" class="patterns-grid">
          <div class="loading-text">Loading patterns...</div>
        </div>
      </div>
    </div>

    <script>
      !(function (e, t) {
        'function' == typeof define && define.amd
          ? define([], t)
          : 'object' == typeof exports
          ? (module.exports = t())
          : (e.SuperGif = t());
      })(this, function () {
        var e = function (e) {
            return e.reduce(function (e, t) {
              return 2 * e + t;
            }, 0);
          },
          t = function (e) {
            for (var t = [], n = 7; n >= 0; n--) t.push(!!(e & (1 << n)));
            return t;
          },
          n = function (e) {
            (this.data = e),
              (this.len = this.data.length),
              (this.pos = 0),
              (this.readByte = function () {
                if (this.pos >= this.data.length)
                  throw new Error('Attempted to read past end of stream.');
                return e instanceof Uint8Array
                  ? e[this.pos++]
                  : 255 & e.charCodeAt(this.pos++);
              }),
              (this.readBytes = function (e) {
                for (var t = [], n = 0; n < e; n++) t.push(this.readByte());
                return t;
              }),
              (this.read = function (e) {
                for (var t = '', n = 0; n < e; n++)
                  t += String.fromCharCode(this.readByte());
                return t;
              }),
              (this.readUnsigned = function () {
                var e = this.readBytes(2);
                return (e[1] << 8) + e[0];
              });
          },
          r = function (n, r) {
            r || (r = {});
            var i = function (e) {
                for (var t = [], r = 0; r < e; r++) t.push(n.readBytes(3));
                return t;
              },
              a = function () {
                var e, t;
                t = '';
                do {
                  (e = n.readByte()), (t += n.read(e));
                } while (0 !== e);
                return t;
              },
              o = function (o) {
                (o.leftPos = n.readUnsigned()),
                  (o.topPos = n.readUnsigned()),
                  (o.width = n.readUnsigned()),
                  (o.height = n.readUnsigned());
                var l = t(n.readByte());
                (o.lctFlag = l.shift()),
                  (o.interlaced = l.shift()),
                  (o.sorted = l.shift()),
                  (o.reserved = l.splice(0, 2)),
                  (o.lctSize = e(l.splice(0, 3))),
                  o.lctFlag && (o.lct = i(1 << (o.lctSize + 1))),
                  (o.lzwMinCodeSize = n.readByte());
                var s = a();
                (o.pixels = (function (e, t) {
                  for (
                    var n,
                      r,
                      i = 0,
                      a = function (e) {
                        for (var n = 0, r = 0; r < e; r++)
                          t.charCodeAt(i >> 3) & (1 << (7 & i)) &&
                            (n |= 1 << r),
                            i++;
                        return n;
                      },
                      o = [],
                      l = 1 << e,
                      s = l + 1,
                      c = e + 1,
                      u = [],
                      d = function () {
                        (u = []), (c = e + 1);
                        for (var t = 0; t < l; t++) u[t] = [t];
                        (u[l] = []), (u[s] = null);
                      };
                    ;

                  )
                    if (((r = n), (n = a(c)) !== l)) {
                      if (n === s) break;
                      if (n < u.length) r !== l && u.push(u[r].concat(u[n][0]));
                      else {
                        if (n !== u.length)
                          throw new Error('Invalid LZW code.');
                        u.push(u[r].concat(u[r][0]));
                      }
                      o.push.apply(o, u[n]),
                        u.length === 1 << c && c < 12 && c++;
                    } else d();
                  return o;
                })(o.lzwMinCodeSize, s)),
                  o.interlaced &&
                    (o.pixels = (function (e, t) {
                      for (
                        var n = new Array(e.length),
                          r = e.length / t,
                          i = function (r, i) {
                            var a = e.slice(i * t, (i + 1) * t);
                            n.splice.apply(n, [r * t, t].concat(a));
                          },
                          a = [0, 4, 2, 1],
                          o = [8, 8, 4, 2],
                          l = 0,
                          s = 0;
                        s < 4;
                        s++
                      )
                        for (var c = a[s]; c < r; c += o[s]) i(c, l), l++;
                      return n;
                    })(o.pixels, o.width)),
                  r.img && r.img(o);
              },
              l = function () {
                var i = {};
                switch (
                  ((i.sentinel = n.readByte()), String.fromCharCode(i.sentinel))
                ) {
                  case '!':
                    (i.type = 'ext'),
                      (function (i) {
                        switch (((i.label = n.readByte()), i.label)) {
                          case 249:
                            (i.extType = 'gce'),
                              (function (i) {
                                n.readByte();
                                var a = t(n.readByte());
                                (i.reserved = a.splice(0, 3)),
                                  (i.disposalMethod = e(a.splice(0, 3))),
                                  (i.userInput = a.shift()),
                                  (i.transparencyGiven = a.shift()),
                                  (i.delayTime = n.readUnsigned()),
                                  (i.transparencyIndex = n.readByte()),
                                  (i.terminator = n.readByte()),
                                  r.gce && r.gce(i);
                              })(i);
                            break;
                          case 254:
                            (i.extType = 'com'),
                              (function (e) {
                                (e.comment = a()), r.com && r.com(e);
                              })(i);
                            break;
                          case 1:
                            (i.extType = 'pte'),
                              (function (e) {
                                n.readByte(),
                                  (e.ptHeader = n.readBytes(12)),
                                  (e.ptData = a()),
                                  r.pte && r.pte(e);
                              })(i);
                            break;
                          case 255:
                            (i.extType = 'app'),
                              (function (e) {
                                n.readByte(),
                                  (e.identifier = n.read(8)),
                                  (e.authCode = n.read(3)),
                                  'NETSCAPE' === e.identifier
                                    ? (function (e) {
                                        n.readByte(),
                                          (e.unknown = n.readByte()),
                                          (e.iterations = n.readUnsigned()),
                                          (e.terminator = n.readByte()),
                                          r.app &&
                                            r.app.NETSCAPE &&
                                            r.app.NETSCAPE(e);
                                      })(e)
                                    : (function (e) {
                                        (e.appData = a()),
                                          r.app &&
                                            r.app[e.identifier] &&
                                            r.app[e.identifier](e);
                                      })(e);
                              })(i);
                            break;
                          default:
                            (i.extType = 'unknown'),
                              (function (e) {
                                (e.data = a()), r.unknown && r.unknown(e);
                              })(i);
                        }
                      })(i);
                    break;
                  case ',':
                    (i.type = 'img'), o(i);
                    break;
                  case ';':
                    (i.type = 'eof'), r.eof && r.eof(i);
                    break;
                  default:
                    throw new Error(
                      'Unknown block: 0x' + i.sentinel.toString(16)
                    );
                }
                'eof' !== i.type && setTimeout(l, 0);
              };
            !(function () {
              var a = {};
              if (((a.sig = n.read(3)), (a.ver = n.read(3)), 'GIF' !== a.sig))
                throw new Error('Not a GIF file.');
              (a.width = n.readUnsigned()), (a.height = n.readUnsigned());
              var o = t(n.readByte());
              (a.gctFlag = o.shift()),
                (a.colorRes = e(o.splice(0, 3))),
                (a.sorted = o.shift()),
                (a.gctSize = e(o.splice(0, 3))),
                (a.bgColor = n.readByte()),
                (a.pixelAspectRatio = n.readByte()),
                a.gctFlag && (a.gct = i(1 << (a.gctSize + 1))),
                r.hdr && r.hdr(a);
            })(),
              setTimeout(l, 0);
          };
        return function (e) {
          var t,
            i,
            a = {
              vp_l: 0,
              vp_t: 0,
              vp_w: null,
              vp_h: null,
              c_w: null,
              c_h: null,
            };
          for (var o in e) a[o] = e[o];
          a.vp_w && a.vp_h && (a.is_vp = !0);
          var l = null,
            s = !1,
            c = null,
            u = null,
            d = null,
            h = null,
            p = null,
            f = null,
            g = null,
            _ = !0,
            y = !1,
            w = [],
            v = [],
            m = a.gif;
          void 0 === a.auto_play &&
            (a.auto_play =
              !m.getAttribute('rel:auto_play') ||
              '1' == m.getAttribute('rel:auto_play'));
          var x,
            b,
            T,
            B,
            C = a.hasOwnProperty('on_end') ? a.on_end : null,
            P = a.hasOwnProperty('loop_delay') ? a.loop_delay : 0,
            S = a.hasOwnProperty('loop_mode') ? a.loop_mode : 'auto',
            k = !a.hasOwnProperty('draw_while_loading') || a.draw_while_loading,
            A =
              !!k &&
              (!a.hasOwnProperty('show_progress_bar') || a.show_progress_bar),
            E = a.hasOwnProperty('progressbar_height')
              ? a.progressbar_height
              : 25,
            I = a.hasOwnProperty('progressbar_background_color')
              ? a.progressbar_background_color
              : 'rgba(255,255,255,0.4)',
            U = a.hasOwnProperty('progressbar_foreground_color')
              ? a.progressbar_foreground_color
              : 'rgba(255,0,22,.8)',
            O = function () {
              (c = null), (u = null), (p = d), (d = null), (f = null);
            },
            R = function () {
              try {
                r(t, H);
              } catch (e) {
                D('parse');
              }
            },
            z = function (e, t) {
              (x.width = e * L()),
                (x.height = t * L()),
                (T.style.minWidth = e * L() + 'px'),
                (B.width = e),
                (B.height = t),
                (B.style.width = e + 'px'),
                (B.style.height = t + 'px'),
                B.getContext('2d').setTransform(1, 0, 0, 1, 0, 0);
            },
            N = function (e, t, n) {
              if (n && A) {
                var r,
                  i,
                  o,
                  l = E;
                if (a.is_vp)
                  y
                    ? ((i = (a.vp_t + a.vp_h - l) / L()),
                      (l /= L()),
                      (r = a.vp_l / L() + (e / t) * (a.vp_w / L())),
                      (o = x.width / L()))
                    : ((i = a.vp_t + a.vp_h - l),
                      (r = a.vp_l + (e / t) * a.vp_w),
                      (o = x.width));
                else
                  (i = (x.height - l) / (y ? L() : 1)),
                    (r = ((e / t) * x.width) / (y ? L() : 1)),
                    (o = x.width / (y ? L() : 1)),
                    (l /= y ? L() : 1);
                (b.fillStyle = I),
                  b.fillRect(r, i, o - r, l),
                  (b.fillStyle = U),
                  b.fillRect(0, i, r, l);
              }
            },
            D = function (e) {
              (l = e),
                (i = { width: m.width, height: m.height }),
                (w = []),
                (b.fillStyle = 'black'),
                b.fillRect(
                  0,
                  0,
                  a.c_w ? a.c_w : i.width,
                  a.c_h ? a.c_h : i.height
                ),
                (b.strokeStyle = 'red'),
                (b.lineWidth = 3),
                b.moveTo(0, 0),
                b.lineTo(a.c_w ? a.c_w : i.width, a.c_h ? a.c_h : i.height),
                b.moveTo(0, a.c_h ? a.c_h : i.height),
                b.lineTo(a.c_w ? a.c_w : i.width, 0),
                b.stroke();
            },
            F = function () {
              f &&
                (w.push({
                  data: f.getImageData(0, 0, i.width, i.height),
                  delay: u,
                }),
                v.push({ x: 0, y: 0 }));
            },
            M = (function () {
              var e,
                t,
                n,
                r = -1,
                i = 0,
                o = function (e) {
                  (r += e), c();
                },
                s =
                  ((e = !1),
                  (t = function () {
                    null !== C && C(m),
                      i++,
                      !1 !== S || i < 0 ? n() : ((e = !1), (_ = !1));
                  }),
                  (n = function () {
                    if ((e = _)) {
                      o(1);
                      var i = 10 * w[r].delay;
                      i || (i = 100),
                        0 == (r + 1 + w.length) % w.length
                          ? ((i += P), setTimeout(t, i))
                          : setTimeout(n, i);
                    }
                  }),
                  function () {
                    e || setTimeout(n, 0);
                  }),
                c = function () {
                  var e;
                  (r = parseInt(r, 10)) > w.length - 1 && (r = 0),
                    r < 0 && (r = 0),
                    (e = v[r]),
                    B.getContext('2d').putImageData(w[r].data, e.x, e.y),
                    (b.globalCompositeOperation = 'copy'),
                    b.drawImage(B, 0, 0);
                };
              return {
                init: function () {
                  l ||
                    ((a.c_w && a.c_h) || b.scale(L(), L()),
                    a.auto_play ? s() : ((r = 0), c()));
                },
                step: s,
                play: function () {
                  (_ = !0), s();
                },
                pause: function () {
                  _ = !1;
                },
                playing: _,
                move_relative: o,
                current_frame: function () {
                  return r;
                },
                length: function () {
                  return w.length;
                },
                move_to: function (e) {
                  (r = e), c();
                },
              };
            })(),
            G = function (e) {
              N(t.pos, t.data.length, e);
            },
            j = function () {},
            W = function (e, t) {
              return function (n) {
                e(n), G(t);
              };
            },
            H = {
              hdr: W(function (e) {
                z((i = e).width, i.height);
              }),
              gce: W(function (e) {
                F(),
                  O(),
                  (c = e.transparencyGiven ? e.transparencyIndex : null),
                  (u = e.delayTime),
                  (d = e.disposalMethod);
              }),
              com: W(j),
              app: { NETSCAPE: W(j) },
              img: W(function (e) {
                f || (f = B.getContext('2d'));
                var t = w.length,
                  n = e.lctFlag ? e.lct : i.gct;
                t > 0 &&
                  (3 === p
                    ? null !== h
                      ? f.putImageData(w[h].data, 0, 0)
                      : f.clearRect(g.leftPos, g.topPos, g.width, g.height)
                    : (h = t - 1),
                  2 === p &&
                    f.clearRect(g.leftPos, g.topPos, g.width, g.height));
                var r = f.getImageData(e.leftPos, e.topPos, e.width, e.height);
                e.pixels.forEach(function (e, t) {
                  e !== c &&
                    ((r.data[4 * t + 0] = n[e][0]),
                    (r.data[4 * t + 1] = n[e][1]),
                    (r.data[4 * t + 2] = n[e][2]),
                    (r.data[4 * t + 3] = 255));
                }),
                  f.putImageData(r, e.leftPos, e.topPos),
                  y || (b.scale(L(), L()), (y = !0)),
                  k && (b.drawImage(B, 0, 0), (k = a.auto_play)),
                  (g = e);
              }, !0),
              eof: function (e) {
                F(),
                  G(!1),
                  (a.c_w && a.c_h) ||
                    ((x.width = i.width * L()), (x.height = i.height * L())),
                  M.init(),
                  (s = !1),
                  X && X(m);
              },
            },
            q = function () {
              var e = m.parentNode,
                t = document.createElement('div');
              (x = document.createElement('canvas')),
                (b = x.getContext('2d')),
                (T = document.createElement('div')),
                (B = document.createElement('canvas')),
                (t.width = x.width = m.width),
                (t.height = x.height = m.height),
                (T.style.minWidth = m.width + 'px'),
                (t.className = 'jsgif'),
                (T.className = 'jsgif_toolbar'),
                t.appendChild(x),
                t.appendChild(T),
                e.insertBefore(t, m),
                e.removeChild(m),
                a.c_w && a.c_h && z(a.c_w, a.c_h),
                (V = !0);
            },
            L = function () {
              return a.max_width && i && i.width > a.max_width
                ? a.max_width / i.width
                : 1;
            },
            V = !1,
            X = !1,
            Z = function (e) {
              return (
                !s &&
                ((X = e || !1),
                (s = !0),
                (w = []),
                O(),
                (h = null),
                (p = null),
                (f = null),
                (g = null),
                !0)
              );
            },
            J = function () {
              return w.reduce(function (e, t) {
                return e + t.delay;
              }, 0);
            };
          return {
            play: M.play,
            pause: M.pause,
            move_relative: M.move_relative,
            move_to: M.move_to,
            get_playing: function () {
              return _;
            },
            get_canvas: function () {
              return x;
            },
            get_canvas_scale: function () {
              return L();
            },
            get_loading: function () {
              return s;
            },
            get_auto_play: function () {
              return a.auto_play;
            },
            get_length: function () {
              return M.length();
            },
            get_frames: function () {
              return w;
            },
            get_duration: function () {
              return J();
            },
            get_duration_ms: function () {
              return 10 * J();
            },
            get_current_frame: function () {
              return M.current_frame();
            },
            load_url: function (e, r) {
              if (Z(r)) {
                var i = new XMLHttpRequest();
                i.open('GET', e, !0),
                  'overrideMimeType' in i
                    ? i.overrideMimeType('text/plain; charset=x-user-defined')
                    : 'responseType' in i
                    ? (i.responseType = 'arraybuffer')
                    : i.setRequestHeader('Accept-Charset', 'x-user-defined'),
                  (i.onloadstart = function () {
                    V || q();
                  }),
                  (i.onload = function (e) {
                    200 != this.status && D('xhr - response'),
                      'response' in this ||
                        (this.response = new VBArray(this.responseText)
                          .toArray()
                          .map(String.fromCharCode)
                          .join(''));
                    var r = this.response;
                    r.toString().indexOf('ArrayBuffer') > 0 &&
                      (r = new Uint8Array(r)),
                      (t = new n(r)),
                      setTimeout(R, 0);
                  }),
                  (i.onprogress = function (e) {
                    e.lengthComputable && N(e.loaded, e.total, !0);
                  }),
                  (i.onerror = function () {
                    D('xhr');
                  }),
                  i.send();
              }
            },
            load: function (e) {
              this.load_url(m.getAttribute('rel:animated_src') || m.src, e);
            },
            load_raw: function (e, r) {
              Z(r) && (V || q(), (t = new n(e)), setTimeout(R, 0));
            },
            set_frame_offset: function (e, t) {
              v[e]
                ? (void 0 !== t.x && (v[e].x = t.x),
                  void 0 !== t.y && (v[e].y = t.y))
                : (v[e] = t);
            },
          };
        };
      });
    </script>
    <script>
      function showStatus(message, isError = false) {
        const status = document.getElementById('status');
        status.textContent = message;
        status.className = 'status ' + (isError ? 'error' : 'success');
        status.style.display = 'block';
        setTimeout(() => {
          status.style.display = 'none';
        }, 3000);
      }

      async function sendCommand(cmd) {
        try {
          const response = await fetch('/command', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ cmd }),
          });

          if (response.ok) {
            if (cmd === 'ping') {
              showStatus('Connection successful! 🎉');
            } else if (cmd === 'music') {
              showStatus('Switched to Music Mode 🎵');
            }
          } else {
            const errorText = await response.text();
            showStatus('Error: ' + errorText, true);
          }
        } catch (error) {
          showStatus('Error: ' + error.message, true);
        }
      }

      async function updateBrightness(value) {
        document.getElementById('brightnessValue').textContent = value;
        try {
          const response = await fetch('/brightness', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ value: parseInt(value) }),
          });
          if (response.ok) {
            showStatus('Brightness updated successfully! 🌟');
          } else {
            const errorText = await response.text();
            showStatus('Error: ' + errorText, true);
          }
        } catch (error) {
          console.error('Brightness error:', error);
        }
      }

      function fileSelected() {
        const file = document.getElementById('gifFile').files[0];
        if (file) {
          document.getElementById('fileName').textContent = file.name;
        }
      }

      async function processGifToFrames(gifFile, onProgress) {
        return new Promise((resolve, reject) => {
          // Create URL for the GIF file
          const gifUrl = URL.createObjectURL(gifFile);
          processGif();

          function processGif() {
            try {
              // Create hidden elements to process the GIF
              const container = document.createElement('div');
              container.style.position = 'absolute';
              container.style.opacity = '0';
              container.style.pointerEvents = 'none';
              container.style.zIndex = '-1000';
              document.body.appendChild(container);

              // Create image element
              const img = document.createElement('img');
              img.src = gifUrl;
              container.appendChild(img);

              // Create canvas for processing frames
              const canvas = document.createElement('canvas');
              canvas.width = 64;
              canvas.height = 64;
              const ctx = canvas.getContext('2d');

              // Initialize SuperGif
              const gif = new window.SuperGif({
                gif: img,
                auto_play: false,
              });

              gif.load(() => {
                try {
                  const frameCount = gif.get_length();
                  const pixelsPerFrame = 64 * 64; // 64x64 matrix

                  // Create a single large buffer to hold all frames
                  const combinedBuffer = new Uint32Array(
                    pixelsPerFrame * frameCount
                  );

                  // Process each frame
                  for (let i = 0; i < frameCount; i++) {
                    // Go to specific frame
                    gif.move_to(i);

                    // Get frame canvas
                    const frameCanvas = gif.get_canvas();

                    // Process the frame and add to combined buffer
                    processSingleFrame(
                      frameCanvas,
                      canvas,
                      ctx,
                      combinedBuffer,
                      i * pixelsPerFrame
                    );

                    // Report progress if callback provided
                    if (onProgress) {
                      onProgress((i + 1) / frameCount);
                    }
                  }

                  // Clean up
                  document.body.removeChild(container);
                  URL.revokeObjectURL(gifUrl);

                  resolve(combinedBuffer);
                } catch (err) {
                  reject(err);
                }
              });
            } catch (err) {
              URL.revokeObjectURL(gifUrl);
              reject(err);
            }
          }
        });
      }

      /**
       * Process a single frame from a GIF
       */
      function processSingleFrame(
        sourceCanvas,
        targetCanvas,
        ctx,
        combinedBuffer,
        bufferOffset
      ) {
        const targetSize = { width: 64, height: 64 };

        // Calculate aspect ratios
        const imgRatio = sourceCanvas.width / sourceCanvas.height;
        const targetRatio = targetSize.width / targetSize.height;

        // Calculate scaling factor
        let scaleFactor;
        if (imgRatio > targetRatio) {
          scaleFactor = targetSize.height / sourceCanvas.height;
        } else {
          scaleFactor = targetSize.width / sourceCanvas.width;
        }

        // Calculate new size
        const newSize = {
          width: Math.floor(sourceCanvas.width * scaleFactor),
          height: Math.floor(sourceCanvas.height * scaleFactor),
        };

        // Clear canvas with black background
        ctx.fillStyle = 'black';
        ctx.fillRect(0, 0, targetSize.width, targetSize.height);

        // Center the image
        const left = Math.floor((targetSize.width - newSize.width) / 2);
        const top = Math.floor((targetSize.height - newSize.height) / 2);

        // Draw the frame
        ctx.drawImage(
          sourceCanvas,
          0,
          0,
          sourceCanvas.width,
          sourceCanvas.height,
          left,
          top,
          newSize.width,
          newSize.height
        );

        // Get pixel data
        const imageData = ctx.getImageData(
          0,
          0,
          targetSize.width,
          targetSize.height
        );
        const pixels = imageData.data;

        // Convert to buffer format and add to the combined buffer at specified offset
        for (let y = 0; y < targetSize.height; y++) {
          for (let x = 0; x < targetSize.width; x++) {
            const pixelIndex = y * targetSize.width + x;
            const dataIndex = pixelIndex * 4;

            const r = pixels[dataIndex];
            const g = pixels[dataIndex + 1];
            const b = pixels[dataIndex + 2];

            // Store directly in the combined buffer at the right offset
            combinedBuffer[bufferOffset + pixelIndex] =
              (r << 16) | (g << 8) | b;
          }
        }
      }

      async function uploadGif() {
        const fileInput = document.getElementById('gifFile');
        const file = fileInput.files[0];

        if (!file) {
          showStatus('Please select a file first', true);
          return;
        }

        const progressBar = document.getElementById('progressBar');
        const progressFill = document.getElementById('progressFill');
        progressBar.style.display = 'block';

        try {
          const frames = await processGifToFrames(file, (progress) => {
            console.log(`Processing GIF: ${Math.round(progress * 100)}%`);
          });

          if (frames.length === 0) {
            throw new Error('No frames were extracted from the GIF');
          }
          console.log(frames);
          const response = await fetch('/gif', {
            method: 'POST',
            headers: { 'Content-Type': 'application/octet-stream' },
            body: frames.buffer,
          });

          if (response.ok) {
            progressFill.style.width = '100%';
            showStatus('GIF uploaded successfully! 🎨');
            setTimeout(() => {
              progressBar.style.display = 'none';
              progressFill.style.width = '0%';
            }, 1000);
          } else {
            throw new Error(await response.text());
          }
        } catch (error) {
          showStatus('Upload failed: ' + error.message, true);
          progressBar.style.display = 'none';
        }
      }

      // Function to load and display patterns
      async function loadPatterns() {
        const container = document.getElementById('patternsContainer');

        try {
          const response = await fetch('/patterns'); // Your getPatternIdsEndpoint

          if (!response.ok) {
            throw new Error('Failed to load patterns');
          }

          const patterns = await response.json();

          // Clear loading text
          container.innerHTML = '';

          if (patterns.length === 0) {
            container.innerHTML =
              '<div class="loading-text">No patterns available</div>';
            return;
          }

          // Create buttons for each pattern
          patterns.forEach((patternId) => {
            const button = document.createElement('button');
            button.className = 'btn-pattern';
            button.textContent = patternId;
            button.onclick = () => selectPattern(patternId);
            container.appendChild(button);
          });
        } catch (error) {
          console.error('Error loading patterns:', error);
          container.innerHTML =
            '<div class="loading-text">Failed to load patterns</div>';
        }
      }

      async function selectPattern(patternId) {
        try {
          const response = await fetch('/command', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ cmd: 'pattern', id: patternId }),
          });

          if (response.ok) {
            showStatus(`Pattern "${patternId}" selected! ✨`);
          } else {
            const errorText = await response.text();
            showStatus('Error: ' + errorText, true);
          }
        } catch (error) {
          showStatus('Error: ' + error.message, true);
        }
      }

      document.addEventListener('DOMContentLoaded', function () {
        loadPatterns();
      });
    </script>
  </body>
</html>
)rawliteral";